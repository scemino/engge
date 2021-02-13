#include <ctime>
#include <random>
#include "engge/Util/RandomNumberGenerator.hpp"

namespace ng {
RandomNumberGenerator::RandomNumberGenerator() {
  time_t t;
  setSeed(static_cast<long>(time(&t)));
}

void RandomNumberGenerator::setSeed(long seed) {
  m_seed = seed;
  srand(m_seed);
}

long RandomNumberGenerator::getSeed() const { return m_seed; }

long RandomNumberGenerator::generateLong(long min, long max) {
  max++;
  auto value = rand() % (max - min) + min;
  return value;
}

float RandomNumberGenerator::generateFloat(float min, float max) {
  float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
  return min + scale * (max - min);       /* [min, max] */
}

}
