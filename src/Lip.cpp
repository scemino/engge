#include <regex>
#include <iostream>
#include <fstream>
#include "Lip.h"
#include "_NGUtil.h"

namespace ng
{
Lip::Lip()
    : _pSettings(nullptr)
{
}

void Lip::setSettings(EngineSettings &settings)
{
    _pSettings = &settings;
}

void Lip::load(const std::string &path)
{
    if (!_pSettings)
        return;

    std::vector<char> buffer;
    _pSettings->readEntry(path, buffer);
    GGPackBufferStream input(buffer);
    _data.clear();
    _path = path;
    std::regex re(R"(^(\d*\.?\d*)\s+(\w)$)");

    std::string line;
    while (getLine(input, line))
    {
        std::smatch matches;
        if (!std::regex_search(line, matches, re))
            continue;

        auto t = std::strtof(matches[1].str().c_str(), nullptr);
        auto text = matches[2].str();
        NGLipData data{sf::seconds(t), text[0]};
        _data.emplace_back(data);
    }
}
} // namespace ng
