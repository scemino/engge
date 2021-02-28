#pragma once
#include <engge/Scripting/VerbExecute.hpp>

namespace ng {
class Engine;
class Entity;

class DefaultVerbExecute final : public VerbExecute {
public:
  explicit DefaultVerbExecute(Engine &engine);

private:
  void execute(const Verb *pVerb, Entity *pObject1, Entity *pObject2) final;
  static bool needsToWalkTo(int verbId, Entity *pObj);
  static bool needsReachAnim(int verbId);
  static bool isFarLook(const Entity *pEntity);
  bool useFlags(Entity *pObject);
  bool callObjectOrActorPreWalk(int verb, Entity *pObj1, Entity *pObj2);
  void getVerb(Entity *pObj, const Verb *&pVerb);

private:
  Engine &m_engine;
};
} // namespace ng
