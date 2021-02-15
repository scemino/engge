#pragma once
#include <functional>
#include <glm/vec2.hpp>
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/Function.hpp>

namespace ng {
class ShakeFunction final : public Function {
public:
  ShakeFunction(std::function<void(const glm::vec2 &)> shake, float amount);
  ~ShakeFunction() final;

  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &elapsed) final;

private:
  std::function<void(const glm::vec2 &)> m_shake;
  float m_amount{0.f};
  float m_shakeTime{0.f};
};
}