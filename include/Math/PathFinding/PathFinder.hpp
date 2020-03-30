#pragma once
#include <vector>
#include "SFML/Graphics.hpp"

namespace ng {
class Walkbox;
class Graph;

class PathFinder {
public:
  explicit PathFinder(const std::vector<Walkbox> &walkboxes);

  std::vector<sf::Vector2f> calculatePath(sf::Vector2f from, sf::Vector2f to);
  [[nodiscard]] std::shared_ptr<Graph> getGraph() const { return _graph; }

private:
  std::shared_ptr<Graph> createGraph();
  bool inLineOfSight(sf::Vector2f start, sf::Vector2f end);

private:
  std::shared_ptr<Graph> _graph;
  const std::vector<Walkbox> &_walkboxes;
};
} // namespace ng