#pragma once
#include <string>
#include <functional>
#include <engge/System/NonCopyable.hpp>
#include <ngf/System/TimeSpan.h>

namespace ng {
class Function : public NonCopyable {
public:
  virtual bool isElapsed() { return true; }
  virtual void operator()(const ngf::TimeSpan &) {}
  virtual ~Function() = default;
};
} // namespace ng
