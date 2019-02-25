#pragma once
#include <vector>
#include "SFML/System.hpp"
#include "EngineSettings.h"

// see https://github.com/DanielSWolf/rhubarb-lip-sync for more details

namespace ng
{
struct NGLipData
{
public:
  sf::Time time;
  char letter;
};

class Lip
{
public:
  Lip();
  void setSettings(EngineSettings &settings);
  void load(const std::string &path);
  const std::vector<NGLipData> getData() const { return _data; }
  std::string getPath() const { return _path; }

private:
  EngineSettings* _pSettings;
  std::string _path;
  std::vector<NGLipData> _data;
};
} // namespace ng
