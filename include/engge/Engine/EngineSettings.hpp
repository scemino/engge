#pragma once
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <ngf/IO/GGPackValue.h>
#include <ngf/IO/GGPack.h>

namespace ng {
class EngineSettings {
public:
  using iterator = std::vector<std::unique_ptr<ngf::GGPack>>::iterator;
  using const_iterator = std::vector<std::unique_ptr<ngf::GGPack>>::const_iterator;

public:
  [[nodiscard]] std::filesystem::path getPath() const;
  void loadPacks();

  [[nodiscard]] int getPackCount() const { return static_cast<int>(m_packs.size()); }

  bool hasEntry(const std::string &name);
  [[nodiscard]] std::vector<char> readBuffer(const std::string &name) const;
  [[nodiscard]] ngf::GGPackValue readEntry(const std::string &name) const;

  iterator begin() { return m_packs.begin(); }
  iterator end() { return m_packs.end(); }

  [[nodiscard]] const_iterator cbegin() const { return m_packs.cbegin(); }
  [[nodiscard]] const_iterator cend() const { return m_packs.cend(); }

private:
  std::vector<std::unique_ptr<ngf::GGPack>> m_packs;
};
} // namespace ng
