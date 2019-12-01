#pragma once
#include <vector>

namespace ng {

class Graph;
struct GraphEdge;

class _AstarAlgorithm {
 private:
    ng::Graph &_graph;
    std::vector<std::shared_ptr<ng::GraphEdge>> SPT;
    std::vector<float> G_Cost; //This array will store the G cost of each node
    std::vector<float> F_Cost; //This array will store the F cost of each node
    std::vector<std::shared_ptr<ng::GraphEdge>> SF;
    int _source;
    int _target;

 public:
    _AstarAlgorithm(ng::Graph &graph, int source, int target);

    [[nodiscard]] std::vector<int> getPath() const;

 private:
    void search();
};
}

