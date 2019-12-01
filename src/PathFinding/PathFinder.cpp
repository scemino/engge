#include <utility>
#include <math.h>
#include <sstream>
#include "_AstarAlgorithm.h"
#include "_IndexedPriorityQueue.h"
#include "Graph.h"
#include "GraphEdge.h"
#include "PathFinder.h"
#include "../_Util.h"

namespace ng
{

PathFinder::PathFinder(const std::vector<Walkbox> &walkboxes)
    : _walkboxes(walkboxes)
{
}

std::vector<std::pair<sf::Vector2i, sf::Vector2i>> _sharedLines;

std::shared_ptr<Graph> PathFinder::createGraph()
{
    _sharedLines.clear();

    for (const auto & w1 : _walkboxes)
    {
        for (const auto & w2 : _walkboxes)
        {
            for (auto i = 0; i < w1.getVertices().size(); i++)
            {
                auto v1 = w1.getVertex(i);
                auto v1p = w2.getVertex((i + 1) % w2.getVertices().size());
                if (v1 == v1p)
                {
                    auto v2 = w1.getVertex((i + 1) % w1.getVertices().size());
                    auto v2p = w2.getVertex(i% w2.getVertices().size());
                    if (v2 == v2p)
                    {
                        _sharedLines.emplace_back(v1, v2);
                    }
                }
            }
        }
    }

    auto mainwalkgraph = std::make_shared<Graph>();
    for (const auto &walkbox : _walkboxes)
    {
        if (walkbox.getVertices().size() <= 2)
            continue;

        for (auto i = 0; i < walkbox.getVertices().size(); i++)
        {
            if (!walkbox.isVertexConcave(i))
                continue;

            auto vertex = walkbox.getVertex(i);
            mainwalkgraph->concaveVertices.push_back(vertex);
            mainwalkgraph->addNode(vertex);
        }
    }
    for (auto i = 0; i < mainwalkgraph->concaveVertices.size(); i++)
    {
        for (auto j = 0; j < mainwalkgraph->concaveVertices.size(); j++)
        {
            auto &c1 = mainwalkgraph->concaveVertices[i];
            auto &c2 = mainwalkgraph->concaveVertices[j];
            if (inLineOfSight(c1, c2))
            {
                mainwalkgraph->addEdge(std::make_shared<GraphEdge>(i, j, distance(c1, c2)));
            }
        }
    }
    return mainwalkgraph;
}

sf::Vector2i PathFinder::getClosestPointOnEdge(const sf::Vector2i &from) const
{
    float minDist = 100000;
    sf::Vector2i closestPoint = from;
    for (const auto &w : _walkboxes)
    {
        float d;
        auto pt = w.getClosestPointOnEdge(from, d);
        if (d < minDist)
        {
            minDist = d;
            closestPoint = pt;
        }
    }
    return closestPoint;
}

bool _isShared(sf::Vector2i v1, sf::Vector2i v2)
{
    for (auto line : _sharedLines)
    {
        if (line.first == v1 && line.second == v2)
        {
            return true;
        }
        if (line.first == v2 && line.second == v1)
        {
            return true;
        }
    }
    return false;
}

std::vector<sf::Vector2i> PathFinder::calculatePath(sf::Vector2i from, sf::Vector2i to)
{
    if (!_graph)
    {
        _graph = createGraph();
    }

    Graph walkgraph(*_graph);
    //create new node on start position
    auto startNodeIndex = static_cast<int>(walkgraph.nodes.size());
    auto it = std::find_if(std::begin(_walkboxes), std::end(_walkboxes), [from](const Walkbox &b) { return b.isEnabled() && b.inside(from); });
    if (it == std::end(_walkboxes))
    {
        from = getClosestPointOnEdge(from);
    }
    it = std::find_if(std::begin(_walkboxes), std::end(_walkboxes), [to](const Walkbox &b) { return b.isEnabled() && b.inside(to); });
    if (it == std::end(_walkboxes))
    {
        to = getClosestPointOnEdge(to);
    }

    walkgraph.addNode(from);

    for (auto i = 0; i < walkgraph.concaveVertices.size(); i++)
    {
        auto c = walkgraph.concaveVertices[i];
        if (inLineOfSight(from, c))
        {
            walkgraph.addEdge(std::make_shared<GraphEdge>(startNodeIndex, i, distance(from, c)));
        }
    }

    //create new node on end position
    int endNodeIndex = static_cast<int>(walkgraph.nodes.size());
    walkgraph.addNode(to);

    for (auto i = 0; i < walkgraph.concaveVertices.size(); i++)
    {
        auto c = walkgraph.concaveVertices[i];
        if (inLineOfSight(to, c))
        {
            auto edge = std::make_shared<GraphEdge>(i, endNodeIndex, distance(to, c));
            walkgraph.addEdge(edge);
        }
    }
    if (inLineOfSight(from, to))
    {
        auto edge = std::make_shared<GraphEdge>(startNodeIndex, endNodeIndex, distance(from, to));
        walkgraph.addEdge(edge);
    }

    _AstarAlgorithm astar(walkgraph, startNodeIndex, endNodeIndex);
    auto indices = astar.getPath();
    std::vector<sf::Vector2i> path;
    path.reserve(indices.size());
    for (auto i : indices)
    {
        path.push_back(walkgraph.nodes[i]);
    }
    return path;
}

bool PathFinder::inLineOfSight(const sf::Vector2i &start, const sf::Vector2i &end)
{
    const float epsilon = 0.5f;
    // Not in LOS if any of the ends is outside the polygon
    auto it = std::find_if(std::begin(_walkboxes), std::end(_walkboxes), [start](const Walkbox &b) { return b.isEnabled() && b.inside(start); });
    if (it == std::end(_walkboxes))
    {
        return false;
    }

    it = std::find_if(std::begin(_walkboxes), std::end(_walkboxes), [end](const Walkbox &b) { return b.isEnabled() && b.inside(end); });
    if (it == std::end(_walkboxes))
    {
        return false;
    }

    // In LOS if it's the same start and end location
    if (length(start - end) < epsilon)
    {
        return true;
    }

    // Not in LOS if any edge is intersected by the start-end line segment
    for (const auto &walkbox : _walkboxes)
    {
        auto size = walkbox.getVertices().size();
        for (size_t i = 0; i < size; i++)
        {
            auto v1 = walkbox.getVertex(i);
            auto v2 = walkbox.getVertex((i + 1) % size);
            if (_isShared(v1, v2) || !lineSegmentsCross(start, end, v1, v2))
                continue;

            //In some cases a 'snapped' endpoint is just a little over the line due to rounding errors. So a 0.5 margin is used to tackle those cases.
            if (Walkbox::distanceToSegment(start, v1, v2) > epsilon && Walkbox::distanceToSegment(end, v1, v2) > epsilon)
            {
                return false;
            }
        }
    }

    // Finally the middle point in the segment determines if in LOS or not
    sf::Vector2i v2 = (start + end) / 2;

    it = std::find_if(std::begin(_walkboxes), std::end(_walkboxes), [v2](const Walkbox &b) { return b.isEnabled() && b.inside(v2); });
    return it != std::end(_walkboxes);
}
} // namespace ng