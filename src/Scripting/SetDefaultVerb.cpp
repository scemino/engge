#include "SetDefaultVerb.hpp"
#include <engge/Engine/Engine.hpp>

namespace ng {
SetDefaultVerb::SetDefaultVerb(Engine &engine) : m_engine(engine) {}

bool SetDefaultVerb::isElapsed() { return m_done; }

void SetDefaultVerb::operator()(const ngf::TimeSpan &) {
  if (m_done)
    return;

  m_done = true;
  m_engine.setDefaultVerb();
}
}