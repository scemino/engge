#pragma once
#include <vector>
#include <list>
#include <iostream>
#include "SFML/Graphics.hpp"
#include "Entity.h"

namespace ng
{
class Walkbox : public sf::Drawable
{
public:
  Walkbox();
  Walkbox(const Walkbox& w);
  explicit Walkbox(std::vector<sf::Vector2i> polygon);
  ~Walkbox() override;

  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }
  void setEnabled(bool isEnabled) { _isEnabled = isEnabled; }
  bool isEnabled() const { return _isEnabled; }

  const std::vector<sf::Vector2i> &getVertices() const { return _polygon; }
  const sf::Vector2i &getVertex(size_t index) const { return _polygon[index]; }

  bool inside(const sf::Vector2i &position, bool toleranceOnOutside = true) const;
  bool isVertexConcave(int vertex) const;
  static float distanceToSegment(const sf::Vector2i &p, const sf::Vector2i &v, const sf::Vector2i &w);
  sf::Vector2i getClosestPointOnEdge(const sf::Vector2i &p3, float& dist) const;

  friend std::ostream &operator<<(std::ostream &os, const Walkbox &walkbox);

private:
  static float distanceToSegmentSquared(const sf::Vector2i &p, const sf::Vector2i &v, const sf::Vector2i &w);
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  std::vector<sf::Vector2i> _polygon;
  std::string _name;
  bool _isEnabled{true};
};
} // namespace ng
