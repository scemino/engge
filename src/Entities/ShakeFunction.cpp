#include "ShakeFunction.hpp"

namespace ng {
ShakeFunction::ShakeFunction(std::function<void(const glm::vec2 &)> shake, float amount)
    : m_shake(std::move(shake)), m_amount(amount) {}

ShakeFunction::~ShakeFunction() = default;

bool ShakeFunction::isElapsed() { return false; }

void ShakeFunction::operator()(const ngf::TimeSpan &elapsed) {
  m_shakeTime += 20.f * elapsed.getTotalSeconds();
  m_shake({m_amount * cosf(m_shakeTime + 0.3f), m_amount * sinf(m_shakeTime)});
}
}