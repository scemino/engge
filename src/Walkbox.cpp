#include "Walkbox.h"

namespace ng
{
Walkbox::Walkbox(const std::vector<sf::Vector2i> &polygon)
    : _polygon(polygon), _isEnabled(true)
{
}

Walkbox::~Walkbox() = default;

void Walkbox::draw(sf::RenderWindow &window, sf::RenderStates states) const
{
    auto color = _isEnabled ? sf::Color::Green : sf::Color::Red;
    sf::VertexArray triangle(sf::LinesStrip, _polygon.size() + 1);
    for (int i = 0; i < _polygon.size(); ++i)
    {
        auto &vertex = _polygon[i];
        triangle[i].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[i].color = color;
    }
    {
        auto &vertex = _polygon[0];
        triangle[_polygon.size()].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[_polygon.size()].color = color;
    }
    window.draw(triangle, states);
}

bool Walkbox::contains(sf::Vector2f pos) const
{
    int i, j, c = 0;
    for (i = 0, j = _polygon.size() - 1; i < _polygon.size(); j = i++)
    {
        if (((_polygon[i].y > pos.y) != (_polygon[j].y > pos.y)) &&
            (pos.x < (_polygon[j].x - _polygon[i].x) * (pos.y - _polygon[i].y) / (_polygon[j].y - _polygon[i].y) + _polygon[i].x))
            c = !c;
    }
    return c != 0;
}
} // namespace ng