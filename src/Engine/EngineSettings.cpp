#include <sstream>
#include <filesystem>
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "../Util/Util.hpp"
namespace fs = std::filesystem;

namespace {
void throwEntryNotFound(const std::string &name) {
  std::string s;
  s = "File '" + name + "' not found in ggpack files.";
  throw std::logic_error(s);
}
}

namespace ng {
std::filesystem::path EngineSettings::getPath() const {
  auto devPath = ng::Locator<ng::Preferences>::get().getUserPreference(PreferenceNames::EnggeDevPath,
                                                                       PreferenceDefaultValues::EnggeDevPath);
  return devPath.empty() ? fs::current_path() : fs::path(devPath);
}

void EngineSettings::loadPacks() {
  auto path = getPath();
  for (const auto &entry : fs::directory_iterator(path)) {
    if (ng::startsWith(entry.path().extension().string(), ".ggpack")) {
      auto pack = std::make_unique<ngf::GGPack>();
      info("Opening pack '{}'...", entry.path().string());
      pack->open(entry.path().string());
      m_packs.push_back(std::move(pack));
    }
  }
}

bool EngineSettings::hasEntry(const std::string &name) {
  std::ifstream is;
  is.open(name);
  if (is.is_open()) {
    is.close();
    return true;
  }
  auto it = std::find_if(m_packs.cbegin(), m_packs.cend(), [&name](const auto &pack) {
    return pack->contains(name);
  });
  return it != m_packs.end();
}

std::vector<char> EngineSettings::readBuffer(const std::string &name) const {
  // first try to find the resource in the filesystem
  std::ifstream is;
  is.open(name);
  if (is.is_open()) {
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    std::vector<char> data;
    data.resize(size);
    is.seekg(0, std::ios::beg);
    is.read(data.data(), size);
    is.close();
    return data;
  }

  // not found in filesystem, check in the pack files
  auto it = std::find_if(m_packs.cbegin(), m_packs.cend(), [&name](const auto &pack) {
    return pack->contains(name);
  });
  if (it != m_packs.end()) {
    return it->get()->readEntry(name);
  }
  throwEntryNotFound(name);
  assert(false);
}

ngf::GGPackValue EngineSettings::readEntry(const std::string &name) const {
  auto it = std::find_if(m_packs.cbegin(), m_packs.cend(), [&name](const auto &pack) {
    return pack->contains(name);
  });
  if (it != m_packs.end()) {
    return it->get()->readHashEntry(name);
  }
  throwEntryNotFound(name);
  assert(false);
}

} // namespace ng
