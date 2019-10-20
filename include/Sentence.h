#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "Function.h"

namespace ng
{
class Sentence : public Function
{
  public:
    Sentence &push_back(std::unique_ptr<Function> func)
    {
        _functions.push_back(std::move(func));
        return *this;
    }

    void stop()
    {
        _stopped = true;
    }

    bool isElapsed() override { return _functions.empty(); }

    void operator()(const sf::Time &elapsed) override
    {
        if (_functions.empty())
            return;
        if(_stopped)
        {
            _functions.clear();
            return;
        }
        if (_functions[0]->isElapsed())
        {
            _functions.erase(_functions.begin());
        }
        else
        {
            (*_functions[0])(elapsed);
        }
    }

  private:
    std::vector<std::unique_ptr<Function>> _functions;
    bool _stopped{false};
};
}