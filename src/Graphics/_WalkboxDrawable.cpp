#include "_WalkboxDrawable.hpp"

namespace ng {
_WalkboxDrawable::_WalkboxDrawable(const ngf::Walkbox &walkbox) : m_walkbox(walkbox) {
}

void _WalkboxDrawable::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  auto height = target.getView().getSize().y;
  auto color = m_walkbox.isEnabled() ? ngf::Colors::Green : ngf::Colors::Red;
  std::vector<ngf::Vertex> vertices(m_walkbox.getVertices().size());
  for (size_t i = 0; i < m_walkbox.getVertices().size(); ++i) {
    auto &vertex = m_walkbox.at(i);
    vertices[i].pos = {vertex.x, height - vertex.y};
    vertices[i].color = color;
  }
  target.draw(ngf::PrimitiveType::LineLoop, vertices, states);
}
}
