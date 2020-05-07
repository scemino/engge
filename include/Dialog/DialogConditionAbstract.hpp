#pragma once
#include <string>

namespace ng {
class DialogConditionAbstract {
public:
  virtual ~DialogConditionAbstract() = default;
  virtual bool isOnce(int32_t line) const = 0;
  virtual bool isShowOnce(int32_t line) const = 0;
  virtual bool isOnceEver(int32_t line) const = 0;
  virtual bool isTempOnce(int32_t line) const = 0;
  virtual bool executeCondition(const std::string &condition) const = 0;
};
}