#pragma once
#include <string>
#include <vector>
#include <ngf/System/TimeSpan.h>

// see https://github.com/DanielSWolf/rhubarb-lip-sync for more details

namespace ng {
struct NGLipData {
public:
  ngf::TimeSpan time;
  char letter;
};

class Lip {
public:
  Lip();

  void clear();
  void load(const std::string &path);
  [[nodiscard]] std::vector<NGLipData> getData() const { return m_data; }
  [[nodiscard]] std::string getPath() const { return m_path; }

private:
  std::string m_path;
  std::vector<NGLipData> m_data;
};
} // namespace ng
