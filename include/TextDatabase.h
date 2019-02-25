#pragma once
#include <string>
#include <map>
#include "EngineSettings.h"

namespace ng
{
class TextDatabase
{
public:
  TextDatabase();

  void setSettings(EngineSettings& settings);
  void load(const std::string &path);
  std::string getText(int id) const
  {
    const auto it = _texts.find(id);
    return it->second;
  }

private:
  std::map<int, std::string> _texts;
  EngineSettings *_pSettings;
};
} // namespace ng
