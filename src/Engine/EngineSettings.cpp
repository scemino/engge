#include "Engine/EngineSettings.hpp"
#include <sstream>

namespace ng
{
EngineSettings::EngineSettings()
{
    _pack1.open("ThimbleweedPark.ggpack1");
    _pack2.open("ThimbleweedPark.ggpack2");
}

bool EngineSettings::hasEntry(const std::string &name)
{
    std::ifstream is;
    is.open(name);
    if (is.is_open())
    {
        is.close();
        return true;
    }
    return _pack1.hasEntry(name) || _pack2.hasEntry(name);
}

void EngineSettings::readEntry(const std::string &name, std::vector<char> &data)
{
    // first try to find the resource in the filesystem
    std::ifstream is;
    is.open(name);
    if (is.is_open())
    {
        is.seekg(0, std::ios::end);
        auto size = is.tellg();
        data.resize(size);
        is.seekg(0, std::ios::beg);
        is.read(data.data(), size);
        is.close();
        return;
    }

    // not found in filesystem, check in the pack files
    if (_pack1.hasEntry(name))
    {
        _pack1.readEntry(name, data);
        return;
    }

    if (_pack2.hasEntry(name))
    {
        _pack2.readEntry(name, data);
    }
}

void EngineSettings::readEntry(const std::string &name, GGPackValue &hash)
{
    if (_pack1.hasEntry(name))
    {
        _pack1.readHashEntry(name, hash);
        return;
    }
    if (_pack2.hasEntry(name))
    {
        _pack2.readHashEntry(name, hash);
    }
}

void EngineSettings::getEntries(std::vector<std::string>& entries)
{
    _pack1.getEntries(entries);
    _pack2.getEntries(entries);
}
} // namespace ng
