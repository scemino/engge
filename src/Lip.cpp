#include <regex>
#include "EngineSettings.h"
#include "Lip.h"
#include "Locator.h"
#include "_Util.h"

namespace ng
{
Lip::Lip() = default;

void Lip::load(const std::string &path)
{
    std::vector<char> buffer;
    Locator<EngineSettings>::get().readEntry(path, buffer);
    GGPackBufferStream input(buffer);
    _data.clear();
    _path = path;
    std::regex re(R"(^(\d*\.?\d*)\s+(\w)$)");

    std::string line;
    while (getLine(input, line) || !line.empty())
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
