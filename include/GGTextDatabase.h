#pragma once
#include <string>
#include <map>

namespace gg
{
class GGTextDatabase
{
public:
  GGTextDatabase();
  void load(const std::string &path);
  std::string getText(int id) { return _texts[id]; }

private:
  std::map<int, std::string> _texts;
};
} // namespace gg
