#include "Math/PathFinding/Graph.hpp"
#include "Math/PathFinding/GraphEdge.hpp"
#include "../../System/_Util.hpp"
#include "_AstarAlgorithm.hpp"
#include "_IndexedPriorityQueue.hpp"

namespace ng {

_AstarAlgorithm::_AstarAlgorithm(ng::Graph &graph, int source, int target)
    : _graph(graph), _source(source), _target(target) {
    G_Cost.resize(_graph.nodes.size(), 0);
    F_Cost.resize(_graph.nodes.size(), 0);
    SPT.resize(_graph.nodes.size(), nullptr);
    SF.resize(_graph.nodes.size(), nullptr);
    search();
}

void _AstarAlgorithm::getPath(std::vector<int>& path) const {
    path.clear();
    if (_target < 0)
        return;
    int nd = _target;
    path.push_back(nd);
    while ((nd!=_source) && (SPT[nd]!=nullptr)) {
        nd = SPT[nd]->from;
        path.push_back(nd);
    }
    std::reverse(path.begin(), path.end());
}

void _AstarAlgorithm::search() {
    ng::_IndexedPriorityQueue pq(F_Cost);
    pq.insert(_source);
    while (!pq.isEmpty()) {
        int NCN = pq.pop();
        SPT[NCN] = SF[NCN];
        if (NCN==_target)
            return;
        auto &edges = _graph.edges[NCN];
        for (auto &edge : edges) {
            float Hcost = ng::length(_graph.nodes[edge->to] - _graph.nodes[_target]);
            float Gcost = G_Cost[NCN] + edge->cost;
            if (SF[edge->to]==nullptr) {
                F_Cost[edge->to] = Gcost + Hcost;
                G_Cost[edge->to] = Gcost;
                pq.insert(edge->to);
                SF[edge->to] = edge;
            } else if ((Gcost < G_Cost[edge->to]) && (SPT[edge->to]==nullptr)) {
                F_Cost[edge->to] = Gcost + Hcost;
                G_Cost[edge->to] = Gcost;
                pq.reorderUp();
                SF[edge->to] = edge;
            }
        }
    }
}
}
