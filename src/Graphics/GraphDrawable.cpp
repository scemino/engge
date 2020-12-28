#include "GraphDrawable.hpp"
#include <ngf/Math/PathFinding/Graph.h>
#include <ngf/Graphics/CircleShape.h>

namespace ng {
GraphDrawable::GraphDrawable(const ngf::Graph &graph, int height)
    : m_graph(graph), m_height(height) {
}

void GraphDrawable::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  ngf::Color color(180, 180, 250);
  std::vector<ngf::Vertex> vertices;
  for (const auto &edge : m_graph.edges) {
    for (const auto &e : edge) {
      auto nodeFrom = (glm::vec2)m_graph.nodes[e->from];
      nodeFrom.y = m_height - nodeFrom.y;
      auto nodeTo = (glm::vec2)m_graph.nodes[e->to];
      nodeTo.y = m_height - nodeTo.y;
      vertices.push_back({nodeFrom, color});
      vertices.push_back({nodeTo, color});
    }
  }
  target.draw(ngf::PrimitiveType::Lines, vertices, states);

  for (auto node : m_graph.nodes) {
    ngf::CircleShape shape(2);
    auto pos = (glm::vec2)node;
    pos.y = m_height - pos.y;
    shape.getTransform().setOrigin({1, 1});
    shape.getTransform().setPosition(pos);
    shape.setColor(color);
    shape.draw(target, states);
  }
}
} // namespace ng
