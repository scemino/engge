#pragma once
#include <vector>
#include "SFML/System.hpp"

// see https://github.com/DanielSWolf/rhubarb-lip-sync for more details

namespace gg
{
struct GGLipData
{
  public:
    sf::Time time;
    char letter;
};

class GGLip
{
  public:
    void load(const std::string &path);
    const std::vector<GGLipData> getData() const { return _data; }

  private:
    std::vector<GGLipData> _data;
};
} // namespace gg
