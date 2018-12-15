#pragma once
#include <string>
#include <map>

namespace ng
{
class NGTextDatabase
{
public:
  NGTextDatabase();
  void load(const std::string &path);
  std::string getText(int id) { return _texts[id]; }

private:
  std::map<int, std::string> _texts;
};
} // namespace ng
