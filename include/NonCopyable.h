#pragma once

namespace gg
{
class NonCopyable
{
protected:
  NonCopyable() {}
  ~NonCopyable() {}

private:
  NonCopyable(const NonCopyable &);
  NonCopyable &operator=(const NonCopyable &);
};
} // namespace gg