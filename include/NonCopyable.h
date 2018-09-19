#pragma once

namespace gg
{
class NonCopyable
{
protected:
  NonCopyable() = default;

  ~NonCopyable() = default;

private:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};
} // namespace gg