#pragma once
#include <string>
#include <squirrel.h>
#include <engge/Scripting/ScriptObject.hpp>

namespace ng {
class ThreadBase : public ScriptObject {
protected:
  ThreadBase();

public:
  ~ThreadBase() override;

  [[nodiscard]] virtual HSQUIRRELVM getThread() const = 0;
  [[nodiscard]] virtual std::string getName() const = 0;

  inline void setPauseable(bool value) { m_isPauseable = value; }
  [[nodiscard]] inline bool isPauseable() const { return m_isPauseable; }
  [[nodiscard]] virtual bool isGlobal() const { return false; }

  void stop();
  bool pause();
  void suspend();
  void resume();

  [[nodiscard]] bool isSuspended() const;
  [[nodiscard]] virtual bool isStopped() const;

private:
  bool m_isSuspended{false};
  bool m_isPauseable{true};
  bool m_isStopped{false};
};
}
