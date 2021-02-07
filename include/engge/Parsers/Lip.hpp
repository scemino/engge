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
  [[nodiscard]] std::vector<NGLipData> getData() const { return _data; }
  [[nodiscard]] std::string getPath() const { return _path; }

private:
  std::string _path;
  std::vector<NGLipData> _data;
};
} // namespace ng
