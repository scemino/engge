#include <sstream>
#include <engge/System/Logger.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Dialog/EngineDialogScript.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Scripting/ScriptEngine.hpp>

namespace ng {

EngineDialogScript::EngineDialogScript(Engine &engine) : m_engine(engine) {}

std::function<bool()> EngineDialogScript::pause(ngf::TimeSpan time) {
  //trace("pause {}", time.asSeconds());
  auto startTime = m_engine.getTime();
  return [*this, startTime, time]() -> bool { return (m_engine.getTime() - startTime) >= time; };
}

std::function<bool()> EngineDialogScript::say(const std::string &actor, const std::string &text) {
  //trace("{}: {}", actor, text);
  auto* pEntity = m_engine.getEntity(actor);

  // is it an animation to play ?
  if (!text.empty() && text[0] == '^') {
    auto pActor = dynamic_cast<Actor*>(pEntity);
    if(pActor) {
      auto anim = text.substr(2, text.length() - 3);
      std::stringstream s;
      s << "actorPlayAnimation(" << pActor->getKey() << ", \"" << anim << "\", NO)";
      m_engine.execute(s.str());
      return []() { return true; };
    }
  }

  // is it a script variable ?
  if (!text.empty() && text[0] == '$') {
    pEntity->say(m_engine.executeDollar(text.substr(1)));
  } else {
    pEntity->say(text);
  }
  return [pEntity]() -> bool { return !pEntity->isTalking(); };
}

void EngineDialogScript::shutup() {
  //trace("shutup");
  m_engine.stopTalking();
}

std::function<bool()> EngineDialogScript::waitFor(const std::string &actor) {
  trace("TODO: waitFor {}", actor);
  return []() {
    return true;
  };
}
std::function<bool()> EngineDialogScript::waitWhile(const std::string &condition) {
  //trace("waitWhile {}", condition);
  return [*this, condition]() -> bool { return !m_engine.executeCondition(condition); };
}
void EngineDialogScript::execute(const std::string &code) {
  //trace("execute {}", code);
  m_engine.execute(code);
}
bool EngineDialogScript::executeCondition(const std::string &condition) const {
  std::string code = condition;
  const auto &actors = m_engine.getActors();
  // check if the code corresponds to an actor name
  auto it = std::find_if(actors.cbegin(), actors.cend(), [&condition](const auto &pActor) {
    return pActor->getKey() == condition;
  });
  if (it != actors.cend()) {
    // yes, so we check if the current actor is the given actor name
    auto pCurrentActor = m_engine.getCurrentActor();
    return pCurrentActor && pCurrentActor->getKey() == (*it)->getKey();
  }

  auto result = m_engine.executeCondition(code);
  //trace("executeCondition {} -> {}", code, result ? "yes" : "no");
  return result;
}
}