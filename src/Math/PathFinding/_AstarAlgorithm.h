#pragma once
#include <vector>
#include <memory>

namespace ng {

class Graph;
struct GraphEdge;

class _AstarAlgorithm {
 private:
    Graph &_graph;
    std::vector<std::shared_ptr<GraphEdge>> SPT; // The Shortest Path Tree
    std::vector<float> G_Cost; //This array will store the G cost of each node
    std::vector<float> F_Cost; //This array will store the F cost of each node
    std::vector<std::shared_ptr<GraphEdge>> SF; // The Search Frontier
    int _source;
    int _target;

 public:
    _AstarAlgorithm(Graph &graph, int source, int target);

    void getPath(std::vector<int>& path) const;

 private:
    void search();
};
}

