#pragma once
#include <cstdint>
#include <string>

namespace ng {
class DialogConditionAbstract {
public:
  virtual ~DialogConditionAbstract() = default;
  [[nodiscard]] virtual bool isOnce(int32_t line) const = 0;
  [[nodiscard]] virtual bool isShowOnce(int32_t line) const = 0;
  [[nodiscard]] virtual bool isOnceEver(int32_t line) const = 0;
  [[nodiscard]] virtual bool isTempOnce(int32_t line) const = 0;
  [[nodiscard]] virtual bool executeCondition(const std::string &condition) const = 0;
};
}