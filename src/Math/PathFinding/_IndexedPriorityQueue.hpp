#pragma once
#include <vector>

namespace ng
{
class _IndexedPriorityQueue {
    std::vector<float> &_keys;
    std::vector<int> _data;

 public:
    explicit _IndexedPriorityQueue(std::vector<float> &keys);

    void insert(int index);
    int pop();

    void reorderUp();

    void reorderDown();

    bool isEmpty();
};
}
