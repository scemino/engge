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
    std::regex re("^(\\d+)\\s+(.*)$");
    std::vector<char> buffer;
    _pSettings->readEntry(path, buffer);
    GGPackBufferStream input(buffer);
    std::string line;
    while (getLine(input, line))
    {
        std::smatch matches;
        if (!std::regex_search(line, matches, re))
            continue;

        char *end;
        auto num = std::strtol(matches[1].str().c_str(), &end, 10);
        auto text = matches[2].str();
        _texts.insert(std::make_pair(num, text));
    }
}
} // namespace ng
