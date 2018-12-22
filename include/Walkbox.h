#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "Entity.h"

namespace ng
{
class Walkbox
{
public:
  explicit Walkbox(const std::vector<sf::Vector2i> &polygon);
  ~Walkbox();

  void draw(sf::RenderWindow &window, sf::RenderStates states) const;
  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }
  bool contains(sf::Vector2f pos) const;
  void setEnabled(bool isEnabled) { _isEnabled = isEnabled; }
  bool isEnabled() const { return _isEnabled; }

private:
  std::vector<sf::Vector2i> _polygon;
  std::string _name;
  bool _isEnabled;
};
} // namespace ng
