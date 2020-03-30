#include <regex>
#include "System/Logger.hpp"
#include "Engine/EngineSettings.hpp"
#include "Engine/TextDatabase.hpp"
#include "../System/_Util.hpp"

namespace ng {
TextDatabase::TextDatabase() = default;

void TextDatabase::load(const std::string &path) {
  _texts.clear();
  std::wregex re(L"^(\\d+)\\s+(.*)$");
  std::vector<char> buffer;
  Locator<EngineSettings>::get().readEntry(path, buffer);
  GGPackBufferStream input(buffer);
  std::wstring line;
  while (getLine(input, line)) {
    std::wsmatch matches;
    if (!std::regex_search(line, matches, re))
      continue;

    wchar_t *end;
    auto num = std::wcstoul(matches[1].str().c_str(), &end, 10);
    auto text = matches[2].str();
    _texts.insert(std::make_pair(num, text));
  }
}

std::wstring TextDatabase::getText(int id) const {
  const auto it = _texts.find(id);
  if (it == _texts.end()) {
    error("Text ID {} doest not exist", id);
    return L"";
  }
  return it->second;
}

} // namespace ng
