#include "../../System/_Util.h"
#include "Math/PathFinding/PathFinder.h"
#include <sstream>
#include <math.h>
#include <utility>
#include "_IndexedPriorityQueue.h"

namespace ng {
_IndexedPriorityQueue::_IndexedPriorityQueue(std::vector<float> &keys)
    : _keys(keys) {
}

void _IndexedPriorityQueue::insert(int index) {
    _data.push_back(index);
    reorderUp();
}

int _IndexedPriorityQueue::pop() {
    int r = _data[0];
    _data[0] = _data[_data.size() - 1];
    _data.pop_back();
    reorderDown();
    return r;
}

void _IndexedPriorityQueue::reorderUp() {
    if (_data.empty())
        return;
    size_t a = _data.size() - 1;
    while (a > 0) {
        if (_keys[_data[a]] >= _keys[_data[a - 1]])
            return;
        int tmp = _data[a];
        _data[a] = _data[a - 1];
        _data[a - 1] = tmp;
        a--;
    }
}

void _IndexedPriorityQueue::reorderDown() {
    if (_data.empty())
        return;
    for (int a = 0; a < _data.size() - 1; a++) {
        if (_keys[_data[a]] <= _keys[_data[a + 1]])
            return;
        int tmp = _data[a];
        _data[a] = _data[a + 1];
        _data[a + 1] = tmp;
    }
}

bool _IndexedPriorityQueue::isEmpty() {
    return _data.empty();
}
}