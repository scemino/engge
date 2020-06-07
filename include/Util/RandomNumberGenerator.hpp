#pragma once

namespace ng {
class RandomNumberGenerator {
public:
  RandomNumberGenerator();
  void setSeed(long seed);
  long getSeed() const;

  long generateLong(long min, long max);
  float generateFloat(float min, float max);

private:
  long _seed{0};
};
}