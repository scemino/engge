#pragma once
#include <string>

namespace ng {
class TextureTools final {
public:
  void render();

public:
  bool texturesVisible{false};

private:
  static std::string convertSize(size_t size);
};
}