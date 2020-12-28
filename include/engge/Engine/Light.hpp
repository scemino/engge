#pragma once
#include <squirrel.h>
#include <ngf/Graphics/Color.h>
#include <glm/vec2.hpp>
#include "engge/Scripting/ScriptObject.hpp"

namespace ng {
class Light : public ScriptObject {
public:
  Light(ngf::Color color, glm::ivec2 pos);
  ~Light() override;

  [[nodiscard]] inline ngf::Color getColor() const { return _color; }
  [[nodiscard]] inline glm::ivec2 getPosition() const { return _pos; }

  inline void setOn(bool on) { _on = on; }
  [[nodiscard]] inline bool isOn() const { return _on; }
  inline void setBrightness(float brightness) { _brightness = brightness; }
  [[nodiscard]] inline float getBrightness() const { return _brightness; }
  inline void setConeDirection(float direction) { _direction = direction; }
  [[nodiscard]] inline float getConeDirection() const { return _direction; }
  inline void setConeAngle(float angle) { _angle = angle; }
  [[nodiscard]] inline float getConeAngle() const { return _angle; }
  inline void setConeFalloff(float falloff) { _falloff = falloff; }
  [[nodiscard]] inline float getConeFalloff() const { return _falloff; }
  inline void setCutOffRadius(float cutoffRadius) { _cutoffRadius = cutoffRadius; }
  [[nodiscard]] inline float getCutOffRadius() const { return _cutoffRadius; }
  inline void setHalfRadius(float halfRadius) { _halfRadius = halfRadius; }
  [[nodiscard]] inline float getHalfRadius() const { return _halfRadius; }
  inline void setZRange(float nearY, float farY) {
    _nearY = nearY;
    _farY = farY;
  }
  inline void getZRange(float &nearY, float &farY) {
    nearY = _nearY;
    farY = _farY;
  }

  inline HSQOBJECT &getTable() { return _table; }

private:
  HSQOBJECT _table{};
  ngf::Color _color;
  glm::ivec2 _pos;
  bool _on{false};
  float _direction{0};
  float _brightness{0};
  float _angle{0};
  float _falloff{0};
  float _cutoffRadius{0};
  float _halfRadius{0};
  float _nearY{0};
  float _farY{0};
};
} // namespace ng