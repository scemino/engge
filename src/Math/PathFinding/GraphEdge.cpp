#include "Math/PathFinding/GraphEdge.hpp"

namespace ng {
GraphEdge::GraphEdge() = default;

GraphEdge::GraphEdge(int from, int to, float cost) {
  this->from = from;
  this->to = to;
  this->cost = cost;
}

std::ostream &operator<<(std::ostream &os, const GraphEdge &edge) {
  return os << '(' << edge.from << ',' << edge.to << ',' << edge.cost << ")";
}
}