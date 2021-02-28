#pragma once
#include <engge/Engine/Function.hpp>

namespace ng {
class Engine;

class SetDefaultVerb final : public Function {
public:
  explicit SetDefaultVerb(Engine &engine);

  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &) final;

private:
  Engine &m_engine;
  bool m_done{false};
};
}