#pragma once
#include <memory>
#include <optional>
#include "Interpolations.hpp"
#include <glm/vec2.hpp>
#include <ngf/System/TimeSpan.h>
#include <ngf/Graphics/Rect.h>

namespace ng {
class Engine;
class Camera {
public:
  Camera();
  virtual ~Camera();

  void panTo(glm::vec2 target, ngf::TimeSpan time, InterpolationMethod interpolation);
  void at(const glm::vec2 &at);
  void move(const glm::vec2 &offset);
  [[nodiscard]] bool isMoving() const;

  void setBounds(const ngf::irect &cameraBounds);
  [[nodiscard]] std::optional<ngf::irect> getBounds() const;
  void resetBounds();

  [[nodiscard]] glm::vec2 getAt() const;

  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
} // namespace ng