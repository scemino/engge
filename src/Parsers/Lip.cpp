#include <regex>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Parsers/Lip.hpp"
#include "engge/System/Locator.hpp"
#include "../Util/Util.hpp"

namespace ng {
Lip::Lip() = default;

void Lip::clear() {
  m_data.clear();
}

void Lip::load(const std::string &path) {
  auto buffer = Locator<EngineSettings>::get().readBuffer(path);
  GGPackBufferStream input(buffer);
  m_data.clear();
  m_path = path;
  std::regex re(R"(^(\d*\.?\d*)\s+(\w)$)");

  std::string line;
  while (getLine(input, line) || !line.empty()) {
    std::smatch matches;
    if (!std::regex_search(line, matches, re))
      continue;

    auto t = std::strtof(matches[1].str().c_str(), nullptr);
    auto text = matches[2].str();
    NGLipData data{ngf::TimeSpan::seconds(t), text[0]};
    m_data.emplace_back(data);
  }
}
} // namespace ng
