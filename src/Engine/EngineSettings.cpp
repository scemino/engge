#include <sstream>
#include <filesystem>
#include "engge/System/Locator.hpp"
#include "engge/System/Logger.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Engine/EngineSettings.hpp"
#include "../System/_Util.hpp"
namespace fs = std::filesystem;

namespace {
void throwEntryNotFound(const std::string &name) {
  std::string s;
  s = "File '" + name + "' not found in ggpack files.";
  throw std::logic_error(s);
}
}

namespace ng {
EngineSettings::EngineSettings() = default;

std::filesystem::path EngineSettings::getPath() const{
  auto devPath = ng::Locator<ng::Preferences>::get().getUserPreference(PreferenceNames::EnggeDevPath,
                                                                       PreferenceDefaultValues::EnggeDevPath);
  return devPath.empty() ? fs::current_path() : fs::path(devPath);
}

void EngineSettings::loadPacks() {
  auto path = getPath();
  for (const auto &entry : fs::directory_iterator(path)) {
    if (ng::startsWith(entry.path().extension().string(), ".ggpack")) {
      auto pack = std::make_unique<GGPack>();
      info("Opening pack '{}'...", entry.path().string());
      pack->open(entry.path().string());
      _packs.push_back(std::move(pack));
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
  auto it = std::find_if(_packs.begin(), _packs.end(), [&name](auto &pack) {
    return pack->hasEntry(name);
  });
  return it != _packs.end();
}

void EngineSettings::readEntry(const std::string &name, std::vector<char> &data) {
  // first try to find the resource in the filesystem
  std::ifstream is;
  is.open(name);
  if (is.is_open()) {
    is.seekg(0, std::ios::end);
    auto size = is.tellg();
    data.resize(size);
    is.seekg(0, std::ios::beg);
    is.read(data.data(), size);
    is.close();
    return;
  }

  // not found in filesystem, check in the pack files
  auto it = std::find_if(_packs.begin(), _packs.end(), [&name](auto &pack) {
    return pack->hasEntry(name);
  });
  if (it != _packs.end()) {
    (*it)->readEntry(name, data);
    return;
  }
  throwEntryNotFound(name);
}

void EngineSettings::readEntry(const std::string &name, GGPackValue &hash) {
  auto it = std::find_if(_packs.begin(), _packs.end(), [&name](auto &pack) {
    return pack->hasEntry(name);
  });
  if (it != _packs.end()) {
    (*it)->readHashEntry(name, hash);
    return;
  }
  throwEntryNotFound(name);
}

void EngineSettings::getEntries(std::vector<std::string> &entries) {
  for (auto &pack : _packs) {
    pack->getEntries(entries);
  }
}
} // namespace ng
