#include <utility>
#include <cmath>
#include "_AstarAlgorithm.h"
#include "Math/PathFinding/Graph.h"
#include "Math/PathFinding/GraphEdge.h"
#include "Math/PathFinding/PathFinder.h"
#include "../../System/_Util.h"

namespace ng {

PathFinder::PathFinder(const std::vector<Walkbox> &walkboxes)
    : _walkboxes(walkboxes) {
}

std::shared_ptr<Graph> PathFinder::createGraph() {
    auto mainWalkgraph = std::make_shared<Graph>();
    for (const auto &walkbox : _walkboxes) {
        if (walkbox.getVertices().size() <= 2)
            continue;

        auto isEnabled = walkbox.isEnabled();
        for (size_t i = 0; i < walkbox.getVertices().size(); i++) {
            if (walkbox.isVertexConcave(i) != isEnabled)
                continue;

            const auto& vertex = walkbox.getVertex(i);
            mainWalkgraph->concaveVertices.push_back(vertex);
            mainWalkgraph->addNode((sf::Vector2f)vertex);
        }
    }
    for (size_t i = 0; i < mainWalkgraph->concaveVertices.size(); i++) {
        for (size_t j = 0; j < mainWalkgraph->concaveVertices.size(); j++) {
            auto c1 = (sf::Vector2f)mainWalkgraph->concaveVertices[i];
            auto c2 = (sf::Vector2f)mainWalkgraph->concaveVertices[j];
            if (inLineOfSight(c1, c2)) {
                mainWalkgraph->addEdge(std::make_shared<GraphEdge>(i, j, distance(c1, c2)));
            }
        }
    }
    return mainWalkgraph;
}

std::vector<sf::Vector2f> PathFinder::calculatePath(sf::Vector2f from, sf::Vector2f to) {
    if(_walkboxes.empty()) return std::vector<sf::Vector2f>();
    
    if (!_graph) {
        _graph = createGraph();
    }

    Graph walkgraph(*_graph);
    //create new node on start position
    auto startNodeIndex = static_cast<int>(walkgraph.nodes.size());
    if (!_walkboxes[0].inside(from)) {
        from = _walkboxes[0].getClosestPointOnEdge(from);
    }
    if (!_walkboxes[0].inside(to)) {
        to = _walkboxes[0].getClosestPointOnEdge(to);
    }

    //Are there more polygons? Then check if endpoint is inside oine of them and find closest point on edge
    if (_walkboxes.size() > 1) {
        for (int i=1;i<_walkboxes.size();i++) {
            if (_walkboxes[i].inside(to)) {
                to = _walkboxes[i].getClosestPointOnEdge(to);
                break;
            }
        }
    }

    walkgraph.addNode(from);

    for (int i = 0; i < walkgraph.concaveVertices.size(); i++) {
        auto c = (sf::Vector2f)walkgraph.concaveVertices[i];
        if (inLineOfSight(from, c)) {
            walkgraph.addEdge(std::make_shared<GraphEdge>(startNodeIndex, i, distance(from, c)));
        }
    }

    //create new node on end position
    int endNodeIndex = static_cast<int>(walkgraph.nodes.size());
    walkgraph.addNode(to);

    for (int i = 0; i < walkgraph.concaveVertices.size(); i++) {
        auto c = (sf::Vector2f)walkgraph.concaveVertices[i];
        if (inLineOfSight(to, c)) {
            auto edge = std::make_shared<GraphEdge>(i, endNodeIndex, distance(to, c));
            walkgraph.addEdge(edge);
        }
    }
    if (inLineOfSight(from, to)) {
        auto edge = std::make_shared<GraphEdge>(startNodeIndex, endNodeIndex, distance(from, to));
        walkgraph.addEdge(edge);
    }

    _AstarAlgorithm astar(walkgraph, startNodeIndex, endNodeIndex);
    std::vector<int> indices;
    astar.getPath(indices);
    std::vector<sf::Vector2f> path;
    for (auto i : indices) {
        path.push_back(walkgraph.nodes[i]);
    }
    return path;
}

bool PathFinder::inLineOfSight(sf::Vector2f start, sf::Vector2f end) {
    const float epsilon = 0.5f;

    // Not in LOS if any of the ends is outside the polygon
    if (!_walkboxes[0].inside(start) || !_walkboxes[0].inside(end)) {
        return false;
    }

    // In LOS if it's the same start and end location
    if (length(start - end) < epsilon) {
        return true;
    }

    // Not in LOS if any edge is intersected by the start-end line segment
    for (const auto &walkbox : _walkboxes) {
        auto size = walkbox.getVertices().size();
        for (size_t i = 0; i < size; i++) {
            auto v1 = (sf::Vector2f)walkbox.getVertex(i);
            auto v2 = (sf::Vector2f)walkbox.getVertex((i + 1)%size);
            if (!lineSegmentsCross(start, end, v1, v2))
                continue;

            //In some cases a 'snapped' endpoint is just a little over the line due to rounding errors. So a 0.5 margin is used to tackle those cases.
            if (Walkbox::distanceToSegment(start, v1, v2) > epsilon
                && Walkbox::distanceToSegment(end, v1, v2) > epsilon) {
                return false;
            }
        }
    }

    // Finally the middle point in the segment determines if in LOS or not
    sf::Vector2f v2 = (start + end) / 2.f;
    auto inside = _walkboxes[0].inside(v2);
    for (int i=1;i<_walkboxes.size();i++) {
        if (_walkboxes[i].inside(v2, false)) {
            inside = false;
        }
    }
    return inside;
}
} // namespace ng