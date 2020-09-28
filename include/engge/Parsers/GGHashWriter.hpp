#pragma  once
#include <string>
#include <vector>
#include <iostream>
#include "GGPackValue.hpp"

namespace ng {
class GGHashWriter {
public:
  void writeHash(const GGPackValue &hash, std::ostream &os);

private:
  void _writeHash(const GGPackValue &hash, std::ostream &os);
  void _writeArray(const GGPackValue &hash, std::ostream &os);
  void _writeValue(const GGPackValue &value, std::ostream &os);
  void _writeString(const std::string &key, std::ostream &os);

private:
  std::vector<std::string> _keys;
};
}
