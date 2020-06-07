#include <ctime>
#include <random>
#include "Util/RandomNumberGenerator.hpp"

namespace ng {
RandomNumberGenerator::RandomNumberGenerator() {
  time_t t;
  setSeed(static_cast<long>(time(&t)));
}

void RandomNumberGenerator::setSeed(long seed) {
  _seed = seed;
  srand(_seed);
}

long RandomNumberGenerator::getSeed() const { return _seed; }

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
