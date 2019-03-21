#include <regex>
#include <fstream>
#include "TextDatabase.h"
#include "_NGUtil.h"

namespace ng
{
TextDatabase::TextDatabase()
    : _pSettings(nullptr)
{
}

void TextDatabase::setSettings(EngineSettings &settings)
{
    _pSettings = &settings;
}

void TextDatabase::load(const std::string &path)
{
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
} // namespace ng
