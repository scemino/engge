#pragma once
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/Function.hpp>

namespace ng {
class Actor;
class Engine;
class Entity;
struct Verb;

class VerbExecuteFunction : public Function {
public:
  VerbExecuteFunction(Engine &engine, Actor &actor, Entity *pObject1, Entity *pObject2, const Verb *pVerb);

private:
  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &) final;
  void onPickup();
  bool callVerb();
  bool callVerbGive();
  void callDefaultObjectVerb();
  bool needToExecuteVerb();
  static bool callVerbDefault(Entity *pEntity);

private:
  Engine &m_engine;
  const Verb *m_pVerb{nullptr};
  Entity *m_pObject1{nullptr};
  Entity *m_pObject2{nullptr};
  Actor &m_actor;
  bool m_done{false};
};
}