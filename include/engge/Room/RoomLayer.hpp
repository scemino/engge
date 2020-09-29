#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "engge/Entities/Entity.hpp"

namespace ng {

struct Background {
public:
  Background(const sf::Vector2i &pos, std::string texture, const sf::IntRect &rect)
      : m_pos(pos), m_texture(std::move(texture)), m_rect(rect) {
  }

public:
  sf::Vector2i m_pos;
  std::string m_texture;
  sf::IntRect m_rect;
};

class RoomLayer {
public:
  RoomLayer();
  ~RoomLayer() = default;

  std::vector<Background> &getBackgrounds() { return _backgrounds; }

  void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
  [[nodiscard]] const sf::Vector2f &getParallax() const { return _parallax; }

  void setZOrder(int zsort) { _zsort = zsort; }
  [[nodiscard]] int getZOrder() const { return _zsort; }

  void addEntity(Entity &entity);
  void removeEntity(Entity &entity);

  void setEnabled(bool enabled) { _enabled = enabled; }
  [[nodiscard]] bool isEnabled() const { return _enabled; }

  void draw(sf::RenderTarget &target, sf::RenderStates states) const;
  void drawForeground(sf::RenderTarget &target, sf::RenderStates states) const;
  void update(const sf::Time &elapsed);

private:
  std::vector<Background> _backgrounds;
  std::vector<std::reference_wrapper<Entity>> _entities;
  sf::Vector2f _parallax{1, 1};
  int _zsort{0};
  bool _enabled{true};
};
} // namespace ng
