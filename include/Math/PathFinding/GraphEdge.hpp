#pragma once
#include <vector>
#include <memory>
#include "SFML/Graphics.hpp"

namespace ng {
struct GraphEdge {
    int from{0};
    int to{0};
    float cost{0};

    GraphEdge();
    GraphEdge(int from, int to, float cost);

    friend std::ostream &operator<<(std::ostream &os, const GraphEdge &edge);
};
}
