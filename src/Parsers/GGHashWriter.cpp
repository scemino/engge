#include <sstream>
#include "Parsers/GGHashWriter.hpp"

namespace ng {
void GGHashWriter::_writeHash(const GGPackValue &hash, std::ostream &os) {
  if (hash.type != 2)
    throw std::logic_error("trying to write a non-hash");
  os.write(&hash.type, 1);

  auto n_pairs = static_cast<int32_t>(hash.hash_value.size());
  os.write((char *) &n_pairs, 4);

  for (auto &value: hash.hash_value) {
    int plo_idx_int = static_cast<int>(_keys.size());
    os.write((char *) &plo_idx_int, 4);
    _writeKey(value.first);
    _writeValue(value.second, os);
  }

  // terminate hash
  os.write(&hash.type, 1);
}

void GGHashWriter::_writeArray(const GGPackValue &array, std::ostream &os) {
  if (array.type != 3)
    throw std::logic_error("trying to write a non-array");
  os.write(&array.type, 1);

  auto n_pairs = static_cast<int32_t>(array.array_value.size());
  os.write((char *) &n_pairs, 4);

  for (const auto &value: array.array_value) {
    _writeValue(value, os);
  }

  // terminate array
  os.write(&array.type, 1);
}

void GGHashWriter::_writeKey(const std::string &key) {
  _keys.push_back(key);
}

void GGHashWriter::_writeValue(const GGPackValue &value, std::ostream &os) {
  switch (value.type) {
  case 1:os.write(&value.type, 1);
    break;
  case 2:_writeHash(value, os);
    break;
  case 3:_writeArray(value, os);
    break;
  case 4: {
    os.write(&value.type, 1);
    auto plo_idx_int = static_cast<int32_t>(_keys.size());
    os.write((char *) &plo_idx_int, 4);
    _writeKey(value.string_value);
    break;
  }
  case 5: {
    os.write(&value.type, 1);
    auto plo_idx_int = static_cast<int32_t>(_keys.size());
    os.write((char *) &plo_idx_int, 4);
    std::ostringstream num;
    num << value.int_value;
    _writeKey(num.str());
    break;
  }
  case 6: {
    os.write(&value.type, 1);
    auto plo_idx_int = static_cast<int32_t>(_keys.size());
    os.write((char *) &plo_idx_int, 4);
    std::ostringstream num;
    num << value.double_value;
    _writeKey(num.str());
    break;
  }
  }
}

void GGHashWriter::writeHash(const GGPackValue &hash, std::ostream &os) {
  _keys.clear();
  // off 0: write signature
  int32_t tmp = 0x04030201;
  os.write((char *) &tmp, 4);
  // off 4: ??
  tmp = 0;
  os.write((char *) &tmp, 4);
  // off 8: write future plo
  os.write((char *) &tmp, 4);
  // off 12: write hash
  _writeHash(hash, os);
  // write strings
  auto stringsOffset = static_cast<int32_t>(os.tellp());
  char nullChar = '\0';
  for (const auto &key : _keys) {
    os.write(key.data(), key.length());
    os.write(&nullChar, 1);
  }

  // write pointer list
  auto ploOffset = static_cast<int32_t>(os.tellp());
  char c = 7;
  os.write((char *) &c, 1);
  for (const auto &key : _keys) {
    os.write((char *) &stringsOffset, 4);
    stringsOffset += key.length() + 1;
  }
  stringsOffset = 0xFFFFFFFF;
  os.write((char *) &stringsOffset, 4);

  // change pointer list offset
  os.seekp(8, std::ios_base::beg);
  os.write((char *) &ploOffset, 4);
}
}
