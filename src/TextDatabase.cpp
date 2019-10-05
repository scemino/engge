#include <regex>
#include <fstream>
#include "TextDatabase.h"
#include "_Util.h"

namespace ng
{
TextDatabase::TextDatabase() = default;

void TextDatabase::setSettings(EngineSettings &settings)
{
    _pSettings = &settings;
}

void TextDatabase::load(const std::string &path)
{
    _texts.clear();
    std::wregex re(L"^(\\d+)\\s+(.*)$");
    std::vector<char> buffer;
    _pSettings->readEntry(path, buffer);
    GGPackBufferStream input(buffer);
    std::wstring line;
    while (getLine(input, line))
    {
        std::wsmatch matches;
        if (!std::regex_search(line, matches, re))
            continue;

        wchar_t *end;
        auto num = std::wcstoul(matches[1].str().c_str(), &end, 10);
        auto text = matches[2].str();
        _texts.insert(std::make_pair(num, text));
    }
}

std::wstring TextDatabase::getText(int id) const
{
  const auto it = _texts.find(id);
  return it->second;
}

} // namespace ng
