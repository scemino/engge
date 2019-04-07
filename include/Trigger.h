#pragma once

namespace ng
{
class Trigger
{
public:
  Trigger() {}
  virtual ~Trigger() {}
  virtual void trig() = 0;
};
}
