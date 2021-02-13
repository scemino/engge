#pragma once
#include <memory>
#include <vector>
#include <ngf/System/TimeSpan.h>
#include <engge/Engine/Function.hpp>

namespace ng {
class Sentence : public Function {
public:
  Sentence &push_back(std::unique_ptr<Function> func);
  void stop();
  bool isElapsed() override;
  void operator()(const ngf::TimeSpan &elapsed) override;

private:
  std::vector<std::unique_ptr<Function>> m_functions;
  bool m_stopped{false};
};
}