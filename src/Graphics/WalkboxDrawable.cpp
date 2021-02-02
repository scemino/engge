#include "WalkboxDrawable.hpp"

namespace ng {
WalkboxDrawable::WalkboxDrawable(const ngf::Walkbox &walkbox, float roomHeight)
    : m_walkbox(walkbox), m_roomHeight(roomHeight) {
}

void WalkboxDrawable::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  auto color = m_walkbox.isEnabled() ? ngf::Colors::Green : ngf::Colors::Red;
  std::vector<ngf::Vertex> vertices(m_walkbox.getVertices().size());
  for (size_t i = 0; i < m_walkbox.getVertices().size(); ++i) {
    auto &vertex = m_walkbox.at(i);
    vertices[i].pos = {vertex.x, m_roomHeight - vertex.y};
    vertices[i].color = color;
  }
  target.draw(ngf::PrimitiveType::LineLoop, vertices, states);
}

}
