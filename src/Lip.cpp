#include <regex>
#include <iostream>
#include <fstream>
#include "Lip.h"

namespace ng
{
void Lip::load(const std::string &path)
{
    std::regex re(R"(^(\d*\.?\d*)\s+(\w)$)");
    std::ifstream infile(path);
    if(!infile.good()) return;
    std::string line;
    while (std::getline(infile, line))
    {
        std::smatch matches;
        if (!std::regex_search(line, matches, re))
            continue;

        auto t = std::strtof(matches[1].str().c_str(), nullptr);
        auto text = matches[2].str();
        NGLipData data{.time = sf::seconds(t), .letter = text[0]};
        _data.emplace_back(data);
    }
}
} // namespace ng
