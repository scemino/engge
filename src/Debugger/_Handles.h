#include <unordered_map>

namespace ng
{
template <typename T>
class _Handles
{
private:
    static constexpr int START_HANDLE = 1000;

    int32_t _nextHandle;
    std::unordered_map<int32_t, T> _handleMap;

public:
    _Handles()
    {
        _nextHandle = START_HANDLE;
    }

    void reset()
    {
        _nextHandle = START_HANDLE;
        _handleMap.clear();
    }

    int create(T value)
    {
        auto handle = _nextHandle++;
        _handleMap[handle] = value;
        return handle;
    }

    auto find(int handle)
    {
        return _handleMap.find(handle);
    }

    auto begin()
    {
        return _handleMap.begin();
    }

    auto end()
    {
        return _handleMap.end();
    }
};
}
