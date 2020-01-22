#include "Math/PathFinding/Walkbox.h"
#include "_WalkboxDrawable.h"

namespace ng {
_WalkboxDrawable::_WalkboxDrawable(const Walkbox &walkbox) : _walkbox(walkbox) {
}

void _WalkboxDrawable::draw(sf::RenderTarget &target, sf::RenderStates states) const {
    auto color = _walkbox.isEnabled() ? sf::Color::Green : sf::Color::Red;
    sf::VertexArray triangle(sf::LinesStrip, _walkbox.getVertices().size() + 1);
    for (size_t i = 0; i < _walkbox.getVertices().size(); ++i) {
        auto &vertex = _walkbox.getVertex(i);
        triangle[i].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[i].color = color;
    }
    {
        auto &vertex = _walkbox.getVertex(0);
        triangle[_walkbox.getVertices().size()].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[_walkbox.getVertices().size()].color = color;
    }
    target.draw(triangle, states);
}
}
