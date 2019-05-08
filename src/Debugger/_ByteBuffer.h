#pragma once

namespace ng
{
class _ByteBuffer
{
private:
    std::vector<char> _buffer;

public:
    std::string getString()
    {
        std::string s;
        std::copy_n(_buffer.begin(), size(), std::back_inserter(s));
        return _buffer.data();
    }

    int size()
    {
        return _buffer.size();
    }

    void append(char *b, int length)
    {
        for (size_t i = 0; i < length; i++)
        {
            _buffer.push_back(b[i]);
        }
    }

    std::string removeFirst(int n)
    {
        std::string s;
        std::copy_n(_buffer.begin(), n, std::back_inserter(s));
        _buffer.erase(_buffer.begin(), _buffer.begin() + n);
        return s;
    }
};
}
