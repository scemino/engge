#pragma once
#include <vector>
#include "SFML/Graphics.hpp"

namespace gg
{
class RoomLayer
{
  public:
    explicit RoomLayer() {}
    ~RoomLayer() {}

    std::vector<std::string> &getNames() { return _names; }
    const std::vector<std::string> &getNames() const { return _names; }

    std::vector<sf::Sprite> &getSprites() { return _sprites; }
    const std::vector<sf::Sprite> &getSprites() const { return _sprites; }

    void setParallax(const sf::Vector2f &parallax) { _parallax = parallax; }
    const sf::Vector2f &getParallax() const { return _parallax; }

    void setZsort(int zsort) { _zsort = zsort; }
    int getZsort() const { return _zsort; }

  private:
    std::vector<std::string> _names;
    std::vector<sf::Sprite> _sprites;
    sf::Vector2f _parallax;
    int _zsort;
};
} // namespace gg
