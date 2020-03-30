#pragma once
#include <memory>
#include <optional>
#include "SFML/Graphics.hpp"
#include "Interpolations.hpp"

namespace ng {
class Engine;
class Camera {
public:
  Camera();
  virtual ~Camera();

  void panTo(sf::Vector2f target, sf::Time time, InterpolationMethod interpolation);
  void at(const sf::Vector2f &at);
  void move(const sf::Vector2f &offset);
  [[nodiscard]] bool isMoving() const;

  void setBounds(const sf::IntRect &cameraBounds);
  [[nodiscard]] std::optional<sf::IntRect> getBounds() const;
  void resetBounds();

  [[nodiscard]] sf::Vector2f getAt() const;

  void setEngine(Engine *pEngine);
  void update(const sf::Time &elapsed);

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
} // namespace ng