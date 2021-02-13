#include "engge/Engine/Sentence.hpp"

namespace ng {
Sentence &Sentence::push_back(std::unique_ptr<Function> func) {
  m_functions.push_back(std::move(func));
  return *this;
}

void Sentence::stop() { m_stopped = true; }

bool Sentence::isElapsed() { return m_functions.empty(); }

void Sentence::operator()(const ngf::TimeSpan &elapsed) {
  if (m_functions.empty())
    return;
  if (m_stopped) {
    m_functions.clear();
    return;
  }
  if (m_functions[0]->isElapsed()) {
    m_functions.erase(m_functions.begin());
  } else {
    (*m_functions[0])(elapsed);
  }
}
} // namespace ng
