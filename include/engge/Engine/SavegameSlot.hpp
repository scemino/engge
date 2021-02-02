#pragma once
#include <filesystem>
#include <string>
#include <ngf/System/TimeSpan.h>

namespace ng {
class SavegameSlot {
public:
  int slot{0};
  time_t savetime{};
  ngf::TimeSpan gametime;
  std::filesystem::path path;
  bool easyMode{false};

  [[nodiscard]] std::wstring getSaveTimeString() const;
  [[nodiscard]] std::wstring getGameTimeString() const;
};
}
