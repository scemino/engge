#include "_Path.hpp"

namespace ng {
_Path::_Path(std::vector<sf::Vector2f> path)
    : _path(std::move(path)) {
}

void _Path::draw(sf::RenderTarget &window, sf::RenderStates states) const {
    auto color = sf::Color::Yellow;
    sf::VertexArray lines(sf::LinesStrip, _path.size());
    for (size_t i = 0; i < _path.size(); ++i) {
        auto &node = _path[i];
        lines[i].position = node;
        lines[i].color = color;
        sf::CircleShape shape(1);
        shape.setPosition(node - sf::Vector2f(0.5f, 0.5f));
        shape.setFillColor(color);
        window.draw(shape, states);
    }
    window.draw(lines, states);
}
}
