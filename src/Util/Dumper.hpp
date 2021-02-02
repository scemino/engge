#pragma once
#include "../Dialog/AstDump.hpp"
#include "engge/Parsers/SavegameManager.hpp"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

class Dumper {
public:
  static void dump(const std::string& filename){
    auto filepath = fs::path(filename);
    auto ext = filepath.extension();
    if (ext == ".byack") {
      ng::_AstDump::dump(filename);
    } else if (ext == ".save") {
      ng::GGPackValue hash;
      ng::SavegameManager::loadGame(filename, hash);
      std::cout << hash;
    }
  }
};
