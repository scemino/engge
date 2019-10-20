#pragma once
#include <vector>
#include "SFML/Graphics.hpp"
#include "Function.h"

namespace ng
{
class Sentence : public Function
{
  public:
    Sentence &push_back(std::unique_ptr<Function> func);
    void stop();
    bool isElapsed() override;
    void operator()(const sf::Time &elapsed) override;

  private:
    std::vector<std::unique_ptr<Function>> _functions;
    bool _stopped{false};
};
}