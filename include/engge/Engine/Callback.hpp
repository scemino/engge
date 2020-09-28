#pragma once
#include <string>
#include <vector>
#include <squirrel.h>
#include "Function.hpp"

namespace ng {
class Callback : public TimeFunction {
private:
  bool _callbackDone{false};
  int _id{0};
  std::string _method{};
  HSQOBJECT _arg;

public:
  Callback(int id, sf::Time duration, std::string method, HSQOBJECT arg);
  ~Callback() override;

  [[nodiscard]] int getId() const { return _id; }
  [[nodiscard]] const std::string& getMethod() const { return _method; }
  [[nodiscard]] HSQOBJECT getArgument() const { return _arg; }

private:
  void onElapsed() override;
};
}
