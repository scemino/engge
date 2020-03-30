#pragma once
#include "Function.hpp"
#include "squirrel.h"

namespace ng {
class Callback : public TimeFunction {
private:
  HSQUIRRELVM _v;
  bool _done;
  HSQOBJECT _method;
  int _id;

public:
  Callback(HSQUIRRELVM v, sf::Time duration, HSQOBJECT method);
  [[nodiscard]] int getId() const { return _id; }

private:
  void onElapsed() override;
};
}
