#pragma once
#include "SFML/Graphics.hpp"

namespace ng {
class Costume;

class BlinkState {
public:
  explicit BlinkState(Costume &costume);

  void setRate(float min, float max);
  void update(sf::Time elapsed);

private:
  Costume &_costume;
  double _min{0};
  double _max{0};
  sf::Time _value;
  int32_t _state{-1};
  sf::Time _elapsed;
};
}
