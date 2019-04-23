#pragma once
#include <vector>
#include <memory>
#include "SFML/Graphics.hpp"

namespace ng
{
struct GraphEdge
{
  int from{0};
  int to{0};
  float cost{0};

  GraphEdge();
  GraphEdge(int from, int to, float cost);

  friend std::ostream &operator<<(std::ostream &os, const GraphEdge &edge);
};

/*
 * This class has been ported from http://www.groebelsloot.com/2016/03/13/pathfinding-part-2/
 * and modified
 * Here is the original comment:
 * All pathfinding classes are based on the AactionScript 3 implementation by Eduardo Gonzalez
 * Code is ported to HaXe and modified when needed
 * http://code.tutsplus.com/tutorials/artificial-intelligence-series-part-1-path-finding--active-4439
 */
class Graph : public sf::Drawable
{
public:
  std::vector<sf::Vector2i> nodes;
  std::vector<std::vector<std::shared_ptr<GraphEdge>>> edges;
  std::vector<sf::Vector2i> concaveVertices;

public:
  Graph();
  Graph(const Graph &graph);

  std::shared_ptr<GraphEdge> getEdge(int from, int to);
  int addNode(sf::Vector2i node);
  void addEdge(const std::shared_ptr<GraphEdge>& edge);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

class Path : public sf::Drawable
{
public:
  explicit Path(std::vector<sf::Vector2i> path);

  const std::vector<sf::Vector2i> &getPath() const { return _path; }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  std::vector<sf::Vector2i> _path;
};
} // namespace ng
