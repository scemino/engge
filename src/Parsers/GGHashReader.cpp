#include <sstream>
#include "Parsers/GGHashReader.hpp"
#include "Parsers/GGPackValue.hpp"

namespace ng {
void GGHashReader::readHash(std::vector<char> &data, GGPackValue &hash) {
int sig = 0;
_bufferStream.setBuffer(data);
_bufferStream.read((char *) &sig, 4);

if (sig != 0x04030201)
throw std::logic_error("GGPack directory signature incorrect");

getOffsets();

// read hash
hash.type = 2;
_bufferStream.seek(12);
readHash(hash);
}

void GGHashReader::readHash(GGPackValue &value) {
  char c = 0;
  _bufferStream.read(&c, 1);
  if (c != 2) {
    throw std::logic_error("trying to parse a non-hash");
  }
  int n_pairs = 0;
  _bufferStream.read((char *) &n_pairs, 4);

  for (auto i = 0; i < n_pairs; i++) {
    int key_plo_idx = 0;
    _bufferStream.read((char *) &key_plo_idx, 4);

    std::string hash_key;
    readString(key_plo_idx, hash_key);
    GGPackValue hash_value;
    readValue(hash_value);
    value.hash_value[hash_key] = hash_value;
  }

  _bufferStream.read(&c, 1);
  if (c != 2)
    throw std::logic_error("unterminated hash");
}

void GGHashReader::readValue(GGPackValue &value) {
  _bufferStream.read((char *) &value.type, 1);
  switch (value.type) {
  case 1:
    // null
    return;
  case 2:
    // hash
    _bufferStream.seek(_bufferStream.tell() - 1);
    readHash(value);
    return;
  case 3:
    // array
  {
    int length = 0;
    _bufferStream.read((char *) &length, 4);
    for (int i = 0; i < length; i++) {
      GGPackValue item;
      readValue(item);
      value.array_value.push_back(item);
    }
    char c = 0;
    _bufferStream.read(&c, 1);
    if (c != 3)
      throw std::logic_error("unterminated array");
    return;
  }
  case 4:
    // string
  {
    int plo_idx_int = 0;
    _bufferStream.read((char *) &plo_idx_int, 4);
    readString(plo_idx_int, value.string_value);
    return;
  }
  case 5:
  case 6: {
    // int
    // double
    int plo_idx_int = 0;
    _bufferStream.read((char *) &plo_idx_int, 4);
    std::string num_str;
    readString(plo_idx_int, num_str);
    if (value.type == 5) {
      value.int_value = std::strtol(num_str.data(), nullptr, 10);
      return;
    }
    value.double_value = std::strtod(num_str.data(), nullptr);
    return;
  }
  default: {
    std::stringstream s;
    s << "Not Implemented: value type " << value.type;
    throw std::logic_error(s.str());
  }
  }
}

void GGHashReader::getOffsets() {
  _bufferStream.seek(8);
  // read ptr list offset & point to first file name offset
  int plo = 0;
  _bufferStream.read((char *) &plo, 4);
  if (plo < 12 || plo >= _bufferStream.getLength() - 4)
    throw std::logic_error("GGPack plo out of range");

  char c = 0;
  _bufferStream.seek(plo);
  _bufferStream.read(&c, 1);
  if (c != 7) {
    throw std::logic_error("GGPack cannot find plo");
  }

  _offsets.clear();
  do {
    uint32_t offset;
    _bufferStream.read((char *) &offset, 4);
    if (offset == 0xFFFFFFFF)
      return;
    _offsets.push_back(offset);
  } while (true);
}

void GGHashReader::readString(int offset, std::string &key) {
  auto pos = _bufferStream.tell();
  offset = _offsets[offset];
  auto off = offset;
  _bufferStream.seek(off);
  char c;
  do {
    _bufferStream.read(&c, 1);
    if (c == 0)
      break;
    key.append(&c, 1);
  } while (true);
  _bufferStream.seek(pos);
}
}
