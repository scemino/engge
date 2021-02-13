#pragma once

namespace ng {
class RandomNumberGenerator {
public:
  RandomNumberGenerator();
  void setSeed(long seed);
  [[nodiscard]] long getSeed() const;

  long generateLong(long min, long max);
  float generateFloat(float min, float max);

private:
  long m_seed{0};
};
}