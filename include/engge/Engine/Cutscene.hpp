#pragma once
#include <squirrel.h>
#include "Function.hpp"
#include "ThreadBase.hpp"

namespace ng {
class Actor;
class Engine;
class Room;
class Cutscene final : public ThreadBase, public Function {
public:
  Cutscene(Engine &engine,
           HSQUIRRELVM v,
           HSQOBJECT thread,
           HSQOBJECT closureObj,
           HSQOBJECT closureCutsceneOverrideObj,
           HSQOBJECT envObj);
  ~Cutscene() final;

  [[nodiscard]] HSQUIRRELVM getThread() const final;
  [[nodiscard]] std::string getName() const final;
  [[nodiscard]] bool isGlobal() const final { return true; }
  [[nodiscard]] bool isStopped() const final;

public:
  bool isElapsed() final;
  void operator()(const ngf::TimeSpan &elapsed) final;
  void cutsceneOverride();
  [[nodiscard]] bool hasCutsceneOverride() const { return m_hasCutsceneOverride; }

private:
  void startCutscene();
  void checkEndCutscene();
  void doCutsceneOverride();
  void checkEndCutsceneOverride();
  void endCutscene();

private:
  Engine &m_engine;
  HSQUIRRELVM m_v{};
  HSQOBJECT m_threadCutscene{};
  int m_state{0};
  HSQOBJECT m_closureObj{};
  HSQOBJECT m_closureCutsceneOverrideObj{};
  HSQOBJECT m_envObj{};
  int m_inputState{0};
  bool m_hasCutsceneOverride{false};
};
} // namespace ng