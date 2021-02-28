#include "PostWalk.hpp"
#include <engge/Entities/Object.hpp>
#include <engge/Engine/Sentence.hpp>
#include <engge/Scripting/ScriptEngine.hpp>

namespace ng {
PostWalk::PostWalk(Sentence &sentence, Entity *pObject1, Entity *pObject2, int verb)
    : m_sentence(sentence), m_pObject1(pObject1), m_pObject2(pObject2), m_verb(verb) {
}

bool PostWalk::isElapsed() { return m_done; }

void PostWalk::operator()(const ngf::TimeSpan &) {
  auto *pObj = dynamic_cast<Object *>(m_pObject1);
  auto functionName = pObj ? "objectPostWalk" : "actorPostWalk";
  bool handled = false;
  if (ScriptEngine::rawExists(m_pObject1, functionName)) {
    ScriptEngine::callFunc(handled, m_pObject1, functionName, m_verb, m_pObject1, m_pObject2);
  }
  if (handled) {
    m_sentence.stop();
  }
  m_done = true;
}
}