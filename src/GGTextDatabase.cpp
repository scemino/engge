#include <regex>
#include <fstream>
#include "GGTextDatabase.h"

namespace gg
{
GGTextDatabase::GGTextDatabase()
{
}

void GGTextDatabase::load(const std::string &path)
{
    std::regex re("^(\\d+)\\s+(.*)$");
    std::ifstream infile(path);
    std::string line;
    while (std::getline(infile, line))
    {
        std::smatch matches;
        if (!std::regex_search(line, matches, re))
            continue;

        auto num = std::atoi(matches[1].str().c_str());
        auto text = matches[2].str();
        _texts.insert(std::make_pair(num, text));
    }
}
} // namespace gg
