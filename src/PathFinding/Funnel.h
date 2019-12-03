#pragma once
#include <vector>
#include <SFML/Graphics.hpp>

namespace ng {
class Funnel {
    void makeFunnel(const std::vector<sf::Vector2f>& portals, std::vector<sf::Vector2f>& path);
};
}
