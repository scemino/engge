#include <utility>

#include "Graph.h"

namespace ng
{
GraphEdge::GraphEdge() = default;

GraphEdge::GraphEdge(int from, int to, float cost)
{
    this->from = from;
    this->to = to;
    this->cost = cost;
}

std::ostream &operator<<(std::ostream &os, const GraphEdge &edge)
{
    return os << '(' << edge.from << ',' << edge.to << ',' << edge.cost << ")";
}

Graph::Graph() = default;

Graph::Graph(const Graph &graph)
    : nodes(graph.nodes), edges(graph.edges), concaveVertices(graph.concaveVertices)
{
}

std::shared_ptr<GraphEdge> Graph::getEdge(int from, int to)
{
    for (auto &e : edges[from])
    {
        if (e->to == to)
        {
            return e;
        }
    }
    return nullptr;
}

int Graph::addNode(sf::Vector2i node)
{
    nodes.push_back(node);
    edges.emplace_back();
    return 0;
}

void Graph::addEdge(const std::shared_ptr<GraphEdge>& edge)
{
    if (getEdge(edge->from, edge->to) == nullptr)
    {
        edges[edge->from].push_back(edge);
    }
    if (getEdge(edge->to, edge->from) == nullptr)
    {
        edges[edge->to].push_back(edge);
    }
}

void Graph::draw(sf::RenderTarget &window, sf::RenderStates states) const
{
    sf::Color color(180, 180, 250);
    std::vector<sf::Vertex> vertices;
    for (const auto& edge : edges)
    {
        for (const auto& e : edge)
        {
            auto &nodeFrom = nodes[e->from];
            auto &nodeTo = nodes[e->to];
            vertices.emplace_back((sf::Vector2f)nodeFrom, color);
            vertices.emplace_back((sf::Vector2f)nodeTo, color);
        }
    }
    window.draw(&vertices[0], vertices.size(), sf::Lines, states);

    for (auto node : nodes)
    {
        sf::CircleShape shape(2);
        shape.setOrigin(sf::Vector2f(1, 1));
        shape.setPosition((sf::Vector2f)node);
        shape.setFillColor(color);
        window.draw(shape, states);
    }
}

Path::Path(std::vector<sf::Vector2i> path)
    : _path(std::move(path))
{
}

void Path::draw(sf::RenderTarget &window, sf::RenderStates states) const
{
    auto color = sf::Color::Yellow;
    sf::VertexArray lines(sf::LinesStrip, _path.size());
    for (size_t i = 0; i < _path.size(); ++i)
    {
        auto &node = _path[i];
        lines[i].position = (sf::Vector2f)node;
        lines[i].color = color;
        sf::CircleShape shape(1);
        shape.setPosition((sf::Vector2f)node - sf::Vector2f(0.5f, 0.5f));
        shape.setFillColor(color);
        window.draw(shape, states);
    }
    window.draw(lines, states);
}
} // namespace ng
