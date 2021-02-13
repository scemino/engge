#include <regex>
#include "engge/System/Logger.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Engine/TextDatabase.hpp"
#include "../Util/Util.hpp"

namespace ng {
TextDatabase::TextDatabase() = default;

void TextDatabase::load(const std::string &path) {
  m_texts.clear();
  std::wregex re(L"^(\\d+)\\s+(.*)$");
  auto buffer = Locator<EngineSettings>::get().readBuffer(path);
  GGPackBufferStream input(buffer);
  std::wstring line;
  while (getLine(input, line)) {
    std::wsmatch matches;
    if (!std::regex_search(line, matches, re))
      continue;

    wchar_t *end;
    auto num = std::wcstoul(matches[1].str().c_str(), &end, 10);
    auto text = matches[2].str();
    m_texts.insert(std::make_pair(num, text));
  }
}

std::wstring TextDatabase::getText(int id) const {
  const auto it = m_texts.find(id);
  if (it == m_texts.end()) {
    error("Text ID {} doest not exist", id);
    return L"";
  }
  auto text = it->second;
  replaceAll(text, L"\\\"", L"\"");
  return text;
}

std::wstring TextDatabase::getText(const std::string &text) const {
  if (!text.empty() && text[0] == '@') {
    auto id = std::strtol(text.c_str() + 1, nullptr, 10);
    return getText(id);
  }
  return towstring(text);
}

} // namespace ng
