#pragma once
#include <functional>
#include <glm/vec2.hpp>
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/Function.hpp>

namespace ng {
class JiggleFunction final : public Function {
public:
  JiggleFunction(std::function<void(float)> jiggle, float amount);
  ~JiggleFunction() final;

  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &elapsed) final;

private:
  std::function<void(float)> m_jiggle;
  float m_amount{0.f};
  float m_jiggleTime{0.f};
};
}