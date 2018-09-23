#include "Walkbox.h"

namespace gg
{
Walkbox::Walkbox(const std::vector<sf::Vector2i> &polygon)
    : _polygon(polygon)
{
}

Walkbox::~Walkbox() = default;

void Walkbox::draw(sf::RenderWindow &window, sf::RenderStates states) const
{
    sf::VertexArray triangle(sf::LinesStrip, _polygon.size() + 1);
    for (int i = 0; i < _polygon.size(); ++i)
    {
        auto &vertex = _polygon[i];
        triangle[i].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[i].color = sf::Color::Green;
    }
    {
        auto &vertex = _polygon[0];
        triangle[_polygon.size()].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[_polygon.size()].color = sf::Color::Green;
    }
    window.draw(triangle, states);
}
} // namespace gg