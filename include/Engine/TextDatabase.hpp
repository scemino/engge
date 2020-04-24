#pragma once
#include <string>
#include <map>

namespace ng {
class TextDatabase {
public:
  TextDatabase();

  void load(const std::string &path);
  [[nodiscard]] std::wstring getText(int id) const;
  std::wstring getText(const std::string &text) const;

private:
  std::map<int, std::wstring> _texts;
};
} // namespace ng
