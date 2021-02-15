#include "JiggleFunction.hpp"

namespace ng {
JiggleFunction::JiggleFunction(std::function<void(float)> jiggle, float amount)
    : m_jiggle(std::move(jiggle)), m_amount(amount) {}

JiggleFunction::~JiggleFunction() = default;

bool JiggleFunction::isElapsed() { return false; }

void JiggleFunction::operator()(const ngf::TimeSpan &elapsed) {
  m_jiggleTime += 20.f * elapsed.getTotalSeconds();
  m_jiggle(m_amount * sinf(m_jiggleTime));
}
}