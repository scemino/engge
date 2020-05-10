#include <sstream>
#include <Util/BTEACrypto.hpp>
#include "System/Logger.hpp"
#include "Parsers/GGHashReader.hpp"
#include "Parsers/GGHashWriter.hpp"
#include "Parsers/SavegameManager.hpp"

namespace ng {
static const uint8_t
    _savegameKey[] = {0xF3, 0xED, 0xA4, 0xAE, 0x2A, 0x33, 0xF8, 0xAF, 0xB4, 0xDB, 0xA2, 0xB5, 0x22, 0xA0, 0x4B, 0x9B};

void SavegameManager::loadGame(const std::string &path, GGPackValue &hash) {
  std::ifstream is(path, std::ifstream::binary);
  is.seekg(0, std::ios::end);
  auto size = static_cast<int>(is.tellg());
  is.seekg(0, std::ios::beg);
  std::vector<char> data(size, '\0');
  is.read(data.data(), size);
  is.close();

  const int decSize = size / 4;
  BTEACrypto::decrypt((uint32_t *) &data[0], decSize, (uint32_t *) _savegameKey);

  const int32_t hashData = *(int32_t *) &data[size - 16];
  const int32_t hashCheck = computeHash(data, size - 16);

  if (hashData != hashCheck) {
    warn("Invalid savegame: {}", path);
    return;
  }

  GGHashReader reader;
  reader.readHash(data, hash);
}

void SavegameManager::saveGame(const std::string &path, const GGPackValue &saveGameHash) {
  // save hash
  std::stringstream o;
  GGHashWriter writer;
  writer.writeHash(saveGameHash, o);

  // encode data
  const int fullSize = 500000;
  const int fullSizeAndFooter = fullSize + 16;
  const int32_t marker = 8 - ((fullSize + 9) % 8);

  std::vector<char> buf(fullSizeAndFooter);
  o.read(buf.data(), fullSize);

  // write at the end 16 bytes: hashdata (4 bytes) + savetime (4 bytes) + marker (8 bytes)
  const int32_t hashData = computeHash(buf, fullSize);
  *(int32_t *) &buf[fullSize] = hashData;
  *(int32_t *) &buf[fullSize + 4] = saveGameHash["savetime"].getInt();
  memset(&buf[fullSize + 8], marker, 8);

  // then encode data
  const int decSize = fullSizeAndFooter / 4;
  BTEACrypto::encrypt((uint32_t *) buf.data(), decSize, (uint32_t *) _savegameKey);

  // write data
  std::ofstream os(path, std::ofstream::binary);
  os.write((char *) buf.data(), fullSizeAndFooter);
  os.close();
}

int32_t SavegameManager::computeHash(const std::vector<char> &data, int32_t size) {
  int32_t v10 = 0;
  int32_t v11 = 0x6583463;
  int32_t v12;

  do {
    v12 = *(uint8_t *) &data[v10++];
    v11 += v12;
  } while (v10 < size);
  return v11;
}

void SavegameManager::loadSaveDat(const std::string &path) {
  std::ifstream is(path, std::ifstream::binary);
  is.seekg(0, std::ios::end);
  auto size = is.tellg();
  is.seekg(0, std::ios::beg);
  std::vector<char> data(size, '\0');
  is.read(data.data(), size);
  is.close();

  const int32_t decSize = size / 4;
  const uint8_t
      key[] = {0x93, 0x9D, 0xAB, 0x2A, 0x2A, 0x56, 0xF8, 0xAF, 0xB4, 0xDB, 0xA2, 0xB5, 0x22, 0xA3, 0x4B, 0x2B};

  BTEACrypto::decrypt((uint32_t *) &data[0], decSize, (uint32_t *) key);

//      std::ofstream os("Save.dat.txt", std::ifstream::binary);
//      os.write(data.data(), size);
//      os.close();
}
}
