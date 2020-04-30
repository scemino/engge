#include <sstream>
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
  auto size = static_cast<int32_t>(is.tellg());
  is.seekg(0, std::ios::beg);
  std::vector<char> data(size, '\0');
  is.read(data.data(), size);
  is.close();

  const int32_t decSize = size / -4;
  encodeDecodeSavegameData((uint32_t *) &data[0], decSize, (uint8_t *) _savegameKey);

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
  const int32_t fullSize = 500000;
  const int32_t fullSizeAndFooter = fullSize + 16;
  const int32_t marker = 8 - ((fullSize + 9) % 8);

  std::vector<char> buf(fullSizeAndFooter);
  o.read(buf.data(), fullSize);

  // write at the end 16 bytes: hashdata (4 bytes) + savetime (4 bytes) + marker (8 bytes)
  const int32_t hashData = computeHash(buf, fullSize);
  *(int32_t *) &buf[fullSize] = hashData;
  *(int32_t *) &buf[fullSize + 4] = saveGameHash["savetime"].getInt();
  memset(&buf[fullSize + 8], marker, 8);

  // then encode data
  const int32_t decSize = fullSizeAndFooter / 4;
  encodeDecodeSavegameData((uint32_t *) buf.data(), decSize, (uint8_t *) _savegameKey);

  // write data
  std::ofstream os(path, std::ofstream::binary);
  os.write((char *) buf.data(), fullSizeAndFooter);
  os.close();
}

void SavegameManager::encodeDecodeSavegameData(uint32_t *data, int32_t size, const uint8_t *key) {
  int32_t v3; // ecx
  uint32_t *v4; // edx
  uint32_t v5; // eax
  int32_t v6; // esi
  uint32_t v7; // edi
  uint32_t v8; // ebx
  uint32_t v9; // edx
  int32_t v10; // esi
  int32_t v11; // eax
  uint32_t v12; // edx
  int32_t v13; // esi
  int32_t v14; // eax
  bool v15; // zf
  uint32_t *v16; // edx
  int32_t v17; // edi
  int32_t v18; // ebx
  uint32_t v19; // eax
  uint32_t v20; // ecx
  int32_t v21; // ebx
  int32_t v22; // esi
  int32_t v23; // edi
  uint32_t *v24; // [esp+Ch] [ebp-10h]
  uint32_t *v25; // [esp+Ch] [ebp-10h]
  uint32_t v26; // [esp+10h] [ebp-Ch]
  int32_t i; // [esp+10h] [ebp-Ch]
  int32_t v28; // [esp+14h] [ebp-8h]
  int32_t v29; // [esp+14h] [ebp-8h]
  int32_t v30; // [esp+18h] [ebp-4h]
  uint32_t v31; // [esp+28h] [ebp+Ch]
  int32_t v32; // [esp+28h] [ebp+Ch]

  if (size <= 1) {
    if (size < -1) {
      v16 = data;
      v17 = -size - 1;
      v29 = -size - 1;
      v18 = 0x9E3779B9 * (52 / -size + 6);
      v19 = *data;
      v25 = &data[-size - 1];
      v32 = 0x9E3779B9 * (52 / -size + 6);
      do {
        v20 = v18;
        v21 = v17;
        for (i = (v20 >> 2) & 3; v21; --v21) {
          v22 = v16[v21 - 1];
          v23 = (16 * v22 ^ (v19 >> 3)) + ((v16[v21 - 1] >> 5) ^ 4 * v19);
          v16 = data;
          v16[v21] -= ((v32 ^ v19) + (v22 ^ *(uint32_t *) (key + 4 * (i ^ (v21 & 3))))) ^ v23;
          v19 = data[v21];
        }
        v16 = data;
        *v16 -= ((v32 ^ v19) + (*v25 ^ *(uint32_t *) (key + 4 * (i ^ (v21 & 3))))) ^ ((16 * *v25 ^ (v19 >> 3))
            + ((*v25 >> 5) ^ 4 * v19));
        v15 = v32 == 0x9E3779B9;
        v18 = v32 + 0x61C88647;
        v19 = *data;
        v17 = v29;
        v32 += 0x61C88647;
      } while (!v15);
    }
  } else {
    v3 = 0;
    v4 = data;
    v28 = 52 / size + 6;
    v5 = data[size - 1];
    v6 = size - 1;
    v24 = &data[size - 1];
    v31 = data[size - 1];
    v26 = v6;
    do {
      v7 = 0;
      v30 = v3 - 0x61C88647;
      v8 = ((uint32_t) (v3 - 0x61C88647) >> 2) & 3;
      if (v6) {
        do {
          v9 = v4[v7 + 1];
          v10 = (16 * v31 ^ (v9 >> 3)) + ((v5 >> 5) ^ 4 * v9);
          v11 = (v30 ^ v9) + (v31 ^ *(uint32_t *) (key + 4 * (v8 ^ (v7 & 3))));
          v4 = data;
          v4[v7] += v11 ^ v10;
          v5 = data[v7++];
          v31 = v5;
        } while (v7 < v26);
      }
      v12 = *v4;
      v13 = (16 * v31 ^ (v12 >> 3)) + ((v5 >> 5) ^ 4 * v12);
      v3 -= 0x61C88647;
      v14 = (v30 ^ v12) + (v31 ^ *(uint32_t *) (key + 4 * (v8 ^ (v7 & 3))));
      v4 = data;
      *v24 += v14 ^ v13;
      v15 = v28-- == 1;
      v5 = *v24;
      v6 = v26;
      v31 = *v24;
    } while (!v15);
  }
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

  const int32_t decSize = size / -4;
  const uint8_t
      key[] = {0x93, 0x9D, 0xAB, 0x2A, 0x2A, 0x56, 0xF8, 0xAF, 0xB4, 0xDB, 0xA2, 0xB5, 0x22, 0xA3, 0x4B, 0x2B};

  encodeDecodeSavegameData((uint32_t *) &data[0], decSize, (uint8_t *) key);

//      std::ofstream os("Save.dat.txt", std::ifstream::binary);
//      os.write(data.data(), size);
//      os.close();
}
}
