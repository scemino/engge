#pragma once
#include <string>
#include "Function.hpp"

namespace ng {
class Callback : public TimeFunction {
private:
  bool _callbackDone{false};
  int _id{0};
  std::string _method{};

public:
  Callback(int id, sf::Time duration, std::string method);
  ~Callback() override = default;

  [[nodiscard]] int getId() const { return _id; }
  [[nodiscard]] const std::string& getMethod() const { return _method; }

private:
  void onElapsed() override;
};
}
