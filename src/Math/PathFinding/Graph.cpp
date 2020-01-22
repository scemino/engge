#include "Math/PathFinding/Graph.h"
#include "Math/PathFinding/GraphEdge.h"

namespace ng
{
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

int Graph::addNode(sf::Vector2f node)
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
        auto e = std::make_shared<GraphEdge>(edge->to, edge->from, edge->cost);
        edges[edge->to].push_back(e);
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
} // namespace ng
