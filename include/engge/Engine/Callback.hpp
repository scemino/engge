#pragma once
#include <string>
#include <vector>
#include <squirrel.h>
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/TimeFunction.hpp>

namespace ng {
class Callback final : public TimeFunction {
public:
  Callback(int id, ngf::TimeSpan duration, std::string method, HSQOBJECT arg);
  ~Callback() final;

  [[nodiscard]] int getId() const { return m_id; }
  [[nodiscard]] const std::string& getMethod() const { return m_method; }
  [[nodiscard]] HSQOBJECT getArgument() const { return m_arg; }

private:
  void onElapsed() final;

private:
  bool m_callbackDone{false};
  int m_id{0};
  std::string m_method{};
  HSQOBJECT m_arg;
};
}
