#pragma once
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <ngf/IO/GGPackValue.h>
#include <ngf/IO/GGPack.h>

namespace ng {
class EngineSettings {
private:
  std::vector<std::unique_ptr<ngf::GGPack>> _packs;

public:
  using iterator = std::vector<std::unique_ptr<ngf::GGPack>>::iterator;
  using const_iterator = std::vector<std::unique_ptr<ngf::GGPack>>::const_iterator;

public:
  [[nodiscard]] std::filesystem::path getPath() const;
  void loadPacks();

  [[nodiscard]] int getPackCount() const { return static_cast<int>(_packs.size()); }

  bool hasEntry(const std::string &name);
  [[nodiscard]] std::vector<char> readBuffer(const std::string &name) const;
  [[nodiscard]] ngf::GGPackValue readEntry(const std::string &name) const;

  iterator begin() { return _packs.begin(); }
  iterator end() { return _packs.end(); }

  [[nodiscard]] const_iterator cbegin() const { return _packs.cbegin(); }
  [[nodiscard]] const_iterator cend() const { return _packs.cend(); }
};
} // namespace ng
