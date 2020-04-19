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
  void _writeKey(const std::string &key);

private:
  std::vector <std::string> _keys;
};
}
