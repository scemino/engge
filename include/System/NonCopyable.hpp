#pragma once

namespace ng {
class NonCopyable {
protected:
  NonCopyable() = default;

  ~NonCopyable() = default;

private:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};
} // namespace ng