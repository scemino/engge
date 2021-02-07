#include <engge/System/Logger.hpp>
#include <engge/Entities/Actor.hpp>
#include <engge/Dialog/EngineDialogScript.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Scripting/ScriptEngine.hpp>

namespace ng {

EngineDialogScript::EngineDialogScript(Engine &engine) : _engine(engine) {}

std::function<bool()> EngineDialogScript::pause(ngf::TimeSpan time) {
  //trace("pause {}", time.asSeconds());
  auto startTime = _engine.getTime();
  return [*this, startTime, time]() -> bool { return (_engine.getTime() - startTime) >= time; };
}

std::function<bool()> EngineDialogScript::say(const std::string &actor, const std::string &text) {
  //trace("{}: {}", actor, text);
  auto* pEntity = _engine.getEntity(actor);

  // is it an animation to play ?
  if (!text.empty() && text[0] == '^') {
    auto pActor = dynamic_cast<Actor*>(pEntity);
    if(pActor) {
      auto anim = text.substr(2, text.length() - 3);
      std::stringstream s;
      s << "actorPlayAnimation(" << pActor->getKey() << ", \"" << anim << "\", NO)";
      _engine.execute(s.str());
      return []() { return true; };
    }
  }

  // is it a script variable ?
  if (!text.empty() && text[0] == '$') {
    pEntity->say(_engine.executeDollar(text.substr(1)));
  } else {
    pEntity->say(text);
  }
  return [pEntity]() -> bool { return !pEntity->isTalking(); };
}

void EngineDialogScript::shutup() {
  //trace("shutup");
  _engine.stopTalking();
}

std::function<bool()> EngineDialogScript::waitFor(const std::string &actor) {
  trace("TODO: waitFor {}", actor);
  return []() {
    return true;
  };
}
std::function<bool()> EngineDialogScript::waitWhile(const std::string &condition) {
  //trace("waitWhile {}", condition);
  return [*this, condition]() -> bool { return !_engine.executeCondition(condition); };
}
void EngineDialogScript::execute(const std::string &code) {
  //trace("execute {}", code);
  _engine.execute(code);
}
bool EngineDialogScript::executeCondition(const std::string &condition) const {
  std::string code = condition;
  const auto &actors = _engine.getActors();
  // check if the code corresponds to an actor name
  auto it = std::find_if(actors.cbegin(), actors.cend(), [&condition](const auto &pActor) {
    return pActor->getKey() == condition;
  });
  if (it != actors.cend()) {
    // yes, so we check if the current actor is the given actor name
    auto pCurrentActor = _engine.getCurrentActor();
    return pCurrentActor && pCurrentActor->getKey() == (*it)->getKey();
  }

  auto result = _engine.executeCondition(code);
  //trace("executeCondition {} -> {}", code, result ? "yes" : "no");
  return result;
}
}