#include "_Path.hpp"

namespace ng {
_Path::_Path(std::vector<sf::Vector2f> path)
    : _path(std::move(path)) {
}

void _Path::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  auto height = target.getView().getSize().y;
  auto color = sf::Color::Yellow;
  sf::VertexArray lines(sf::LinesStrip, _path.size());
  for (size_t i = 0; i < _path.size(); ++i) {
    auto &node = _path[i];
    auto pos = sf::Vector2f(node.x, height - node.y);
    lines[i].position = pos;
    lines[i].color = color;
    sf::CircleShape shape(1);
    shape.setPosition(pos - sf::Vector2f(0.5f, 0.5f));
    shape.setFillColor(color);
    target.draw(shape, states);
  }
  target.draw(lines, states);
}
}
