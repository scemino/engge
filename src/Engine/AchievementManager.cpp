#include "AchievementManager.hpp"
#include <engge/Util/BTEACrypto.hpp>
#include <engge/Parsers/SavegameManager.hpp>
#include <ngf/IO/Json/JsonParser.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <vector>

namespace {
const uint8_t
    key[] = {0x93, 0x9D, 0xAB, 0x2A, 0x2A, 0x56, 0xF8, 0xAF, 0xB4, 0xDB, 0xA2, 0xB5, 0x22, 0xA3, 0x4B, 0x2B};

std::string toString(const ngf::GGPackValue &value) {
  std::ostringstream os;
  for (const auto &item : value.items()) {
    os << item.key() << ": ";
    if (item.value().isString()) {
      os << "\"" << item.value().getString() << "\"";
    } else if (item.value().isDouble()) {
      os << item.value().getDouble();
    } else if (item.value().isInteger()) {
      os << item.value().getInt();
    }
    os << '\n';
  }
  return os.str();
}
}

namespace ng {
void AchievementManager::load(const std::filesystem::path &path) {
  std::ifstream is(path, std::ifstream::binary);
  is.seekg(0, std::ios::end);
  auto size = static_cast<int>(is.tellg());
  is.seekg(0, std::ios::beg);
  std::vector<char> data(size, '\0');
  is.read(data.data(), size);
  is.close();

  const int32_t decSize = size / 4;
  BTEACrypto::decrypt((uint32_t *) &data[0], decSize, (uint32_t *) key);

  auto marker = data[size - 1];
  data[size - (marker + 1 + 8)] = 0;

  m_value = ngf::Json::parse(data.data());
//  std::ofstream os("Save.dat.txt", std::ifstream::binary);
//  os.write(data.data(), size);
//  os.close();
}

void AchievementManager::save(const std::filesystem::path &path) {
  auto content = toString(m_value);
  auto fullSize = content.size();
  const int32_t marker = 8 - ((fullSize + 9) % 8);

  std::vector<char> buffer(fullSize + 8 + marker + 1);
  memcpy(buffer.data(), content.data(), fullSize);

  time_t now;
  time(&now);

  // write at the end 16 bytes: hashdata (4 bytes) + savetime (4 bytes) + marker
  *(int32_t *) &buffer[fullSize] = SavegameManager::computeHash(buffer, fullSize);
  *(int32_t *) &buffer[fullSize + 4] = static_cast<int32_t>(now);
  memset(&buffer[fullSize + 8], marker, marker + 1);

  BTEACrypto::encrypt((uint32_t *) buffer.data(), buffer.size() / 4, (uint32_t *) key);

  std::ofstream os(path, std::ifstream::binary);
  os.write(buffer.data(), buffer.size());
  os.close();
}

ngf::GGPackValue AchievementManager::getPrivatePreference(const std::string &name) const {
  return m_value[name];
}
}