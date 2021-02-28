#pragma once
#include <vector>
#include <glm/vec2.hpp>
#include <engge/Engine/Function.hpp>
#include <engge/Entities/Facing.hpp>

namespace ng {
class Actor;
class Engine;
class Entity;
class Sentence;
struct Verb;

class ActorWalk final : public Function {
public:
  ActorWalk(Engine &engine, Actor &actor, Entity *pEntity, Sentence *pSentence, const Verb *pVerb);

private:
  static Facing getFacing(const Entity *pEntity);

private:
  bool isElapsed() final;
  void callDefaultObjectVerb();

private:
  Engine &m_engine;
  Actor &m_actor;
  Entity *m_pEntity{nullptr};
  Sentence *m_pSentence{nullptr};
  const Verb *m_pVerb{nullptr};
  std::vector<glm::vec2> m_path;
  bool m_isDestination{false};
  bool m_done{false};
};
}