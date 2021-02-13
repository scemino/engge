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

  /// @brief Pans the camera to the target position in a specified time using a given interpolation.
  /// \param target Position where the camera needs to go.
  /// \param time Time needed for the camera to reach the target position.
  /// \param interpolation Interpolation method to use between the current position and the target position.
  void panTo(glm::vec2 target, ngf::TimeSpan time, InterpolationMethod interpolation);

  /// @brief Sets the position of the camera.
  /// @details The position is the center of the camera.
  /// \param at Position of the camera to set.
  void at(const glm::vec2 &at);

  /// @brief Gets the position of the camera.
  /// @details The position is the center of the camera.
  /// \return The current position of the camera.
  [[nodiscard]] glm::vec2 getAt() const;

  /// @brief Gets the rectangle of the camera.
  [[nodiscard]] ngf::frect getRect() const;

  void move(const glm::vec2 &offset);
  [[nodiscard]] bool isMoving() const;

  void setBounds(const ngf::irect &cameraBounds);
  [[nodiscard]] std::optional<ngf::irect> getBounds() const;
  void resetBounds();

  void setEngine(Engine *pEngine);
  void update(const ngf::TimeSpan &elapsed);

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
} // namespace ng