#pragma once
#include <string>
#include <map>

namespace ng {
class TextDatabase {
public:
  TextDatabase();

  void load(const std::string &path);
  [[nodiscard]] std::wstring getText(int id) const;
  [[nodiscard]] std::wstring getText(const std::string &text) const;

private:
  std::map<int, std::wstring> m_texts;
};
} // namespace ng
