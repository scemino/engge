#pragma once
#include <string>
#include <map>

namespace ng
{
class TextDatabase
{
public:
  TextDatabase();
  void load(const std::string &path);
  std::string getText(int id) const
  {
    const auto it = _texts.find(id);
    return it->second;
  }

private:
  std::map<int, std::string> _texts;
};
} // namespace ng
