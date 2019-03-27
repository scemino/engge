#pragma once
#include <string>

namespace ng
{
class SoundDefinition;
class ScriptExecute
{
  public:
    virtual ~ScriptExecute() = default;
    virtual void execute(const std::string &code) = 0;
    virtual std::string executeDollar(const std::string &code) = 0;
    virtual bool executeCondition(const std::string &code) = 0;
    virtual SoundDefinition *getSoundDefinition(const std::string &name) = 0;
};
}