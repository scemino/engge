#pragma once
#include <engge/Engine/Function.hpp>

namespace ng {
class Entity;
class Sentence;

class PostWalk final : public Function {
public:
  PostWalk(Sentence &sentence, Entity *pObject1, Entity *pObject2, int verb);

  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &) final;

private:
  Sentence &m_sentence;
  Entity *m_pObject1{nullptr};
  Entity *m_pObject2{nullptr};
  int m_verb{0};
  bool m_done{false};
};
}