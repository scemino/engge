#include <sstream>
#include "engge/Parsers/GGHashReader.hpp"
#include "engge/Parsers/GGPack.hpp"
#include "engge/Parsers/GGPackValue.hpp"

namespace ng {
static const unsigned char _magicBytes[] = {
    0x4f, 0xd0, 0xa0, 0xac, 0x4a, 0x5b, 0xb9, 0xe5, 0x93, 0x79, 0x45, 0xa5, 0xc1, 0xcb, 0x31, 0x93};

GGPack::GGPack() = default;

void GGPack::open(const std::string &path) {
  _input.open(path, std::ios::binary);
  readPack();
}

void GGPack::readHashEntry(const std::string &name, GGPackValue &value) {
  std::vector<char> data;
  readEntry(name, data);

  GGHashReader reader;
  reader.readHash(data, value);
}

void GGPack::readPack() {
  if (!_input.is_open())
    return;

  int dataOffset, dataSize;
  _input.read((char *) &dataOffset, 4);
  _input.read((char *) &dataSize, 4);

  std::vector<char> buf(dataSize);

  // try to detect correct method to decode data
  int sig = 0;
  for (_method = 3; _method >= 0; _method--) {
    _input.seekg(dataOffset, std::ios::beg);
    _input.read(&buf[0], dataSize);
    decodeUnbreakableXor(&buf[0], dataSize);
    sig = *(int *) buf.data();
    if (sig == 0x04030201)
      break;
  }

  if (sig != 0x04030201)
    throw std::logic_error("This version of package is not supported (yet?)");

  // read hash
  _entries.clear();

  GGPackValue entries;
  GGHashReader reader;
  reader.readHash(buf, entries);

  auto len = entries["files"].array_value.size();
  for (size_t i = 0; i < len; i++) {
    auto filename = entries["files"][i]["filename"].string_value;
    GGPackEntry entry{};
    entry.offset = entries["files"][i]["offset"].int_value;
    entry.size = entries["files"][i]["size"].int_value;
    _entries.insert(std::pair<std::string, GGPackEntry>(filename, entry));
  }
}

bool GGPack::hasEntry(const std::string &name) {
  return _entries.find(name) != _entries.end();
}

void GGPack::readEntry(const std::string &name, std::vector<char> &data) {
  auto entry = _entries[name];
  data.resize(entry.size + 1);
  _input.seekg(entry.offset, std::ios::beg);

  _input.read(data.data(), entry.size);
  decodeUnbreakableXor(data.data(), entry.size);
  data[entry.size] = 0;
}

char *GGPack::decodeUnbreakableXor(char *buffer, int length) {
  int code = _method != 2 ? 0x6d : 0xad;
  char previous = length & 0xff;
  for (auto i = 0; i < length; i++) {
    auto x = (char) (buffer[i] ^ _magicBytes[i & 0xf] ^ (i * code));
    buffer[i] = (char) (x ^ previous);
    previous = x;
  }
  if (_method != 0) {
    //Loop through in blocks of 16 and xor the 6th and 7th bytes
    int i = 5;
    while (i < length) {
      buffer[i] = (char) (buffer[i] ^ 0x0d);
      if (i + 1 < length) {
        buffer[i + 1] = (char) (buffer[i + 1] ^ 0x0d);
      }
      i += 16;
    }
  }
  return buffer;
}

void GGPack::getEntries(std::vector<std::string> &entries) {
  for (auto &entry : _entries) {
    entries.push_back(entry.first);
  }
}

} // namespace ng
