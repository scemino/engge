#include <engge/Engine/TimeFunction.hpp>

namespace ng {
TimeFunction::TimeFunction(const ngf::TimeSpan &time)
    : m_time(time) {
}

TimeFunction::~TimeFunction() = default;

void TimeFunction::operator()(const ngf::TimeSpan &elapsed) {
  m_elapsed += elapsed;
}

ngf::TimeSpan TimeFunction::getElapsed() const { return m_elapsed; }

bool TimeFunction::isElapsed() {
  auto isElapsed = m_elapsed > m_time;
  if (isElapsed && !m_done) {
    m_done = true;
    onElapsed();
  }
  return isElapsed;
}

void TimeFunction::onElapsed() {}
}