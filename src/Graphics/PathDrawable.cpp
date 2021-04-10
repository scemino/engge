#include <ngf/Graphics/CircleShape.h>
#include "PathDrawable.hpp"

namespace ng {
PathDrawable::PathDrawable(std::vector<glm::vec2> path)
    : m_path(std::move(path)) {
}

void PathDrawable::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  auto height = target.getView().getSize().y;
  auto color = ngf::Colors::Yellow;
  std::vector<ngf::Vertex> lines(m_path.size());
  for (size_t i = 0; i < m_path.size(); ++i) {
    auto &node = m_path[i];
    auto pos = glm::vec2(node.x, height - node.y);
    lines[i].pos = pos;
    lines[i].color = color;
    ngf::CircleShape shape(1);
    shape.getTransform().setPosition(pos - glm::vec2(0.5f, 0.5f));
    shape.setColor(color);
    shape.draw(target, states);
  }
  target.draw(ngf::PrimitiveType::LineStrip, lines, states);
}
}
