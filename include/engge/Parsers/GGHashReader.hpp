#pragma once
#include <vector>
#include "GGPack.hpp"
#include "GGPackBufferStream.hpp"
#include "GGPackValue.hpp"

namespace ng {
class GGHashReader {
public:
  void readHash(std::vector<char> &data, GGPackValue &hash);

private:
  void readHash(GGPackValue &value);
  void readValue(GGPackValue &value);
  void getOffsets();
  void readString(int offset, std::string &key);

private:
  GGPackBufferStream _bufferStream;
  std::vector<int> _offsets;
};
}
