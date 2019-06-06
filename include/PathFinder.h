#pragma once
#include <vector>
#include "Walkbox.h"
#include "Graph.h"

namespace ng
{
class PathFinder
{
public:
  explicit PathFinder(const std::vector<Walkbox> &walkboxes);

  std::vector<sf::Vector2i> calculatePath(sf::Vector2i from, sf::Vector2i to);
  std::shared_ptr<Graph> getGraph() const { return _graph; }

private:
  std::shared_ptr<Graph> createGraph();
  bool inLineOfSight(const sf::Vector2i &start, const sf::Vector2i &end);
  sf::Vector2i getClosestPointOnEdge(const sf::Vector2i &from) const;

private:
  std::shared_ptr<Graph> _graph;
  const  std::vector<Walkbox>& _walkboxes;
};
} // namespace ng