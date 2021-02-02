#include <sstream>
#include <ngf/IO/GGPackHashReader.h>
#include <ngf/IO/MemoryStream.h>
#include <ngf/IO/GGPackHashWriter.h>
#include "engge/Util/BTEACrypto.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Parsers/SavegameManager.hpp"

namespace ng {
static const uint8_t
    _savegameKey[] = {0xF3, 0xED, 0xA4, 0xAE, 0x2A, 0x33, 0xF8, 0xAF, 0xB4, 0xDB, 0xA2, 0xB5, 0x22, 0xA0, 0x4B, 0x9B};

ngf::GGPackValue SavegameManager::loadGame(const std::filesystem::path &path) {
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
    warn("Invalid savegame: {}", path.string().c_str());
    return nullptr;
  }

  ngf::MemoryStream ms(data.data(), data.data() + data.size());
  return ngf::GGPackHashReader::read(ms);
}

void SavegameManager::saveGame(const std::filesystem::path &path, const ngf::GGPackValue &saveGameHash) {
  // save hash
  std::stringstream o;
  ngf::GGPackHashWriter::write(saveGameHash, o);

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
}
