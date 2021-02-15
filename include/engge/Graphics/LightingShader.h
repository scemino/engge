#pragma once
#include <array>
#include <ngf/Graphics/Color.h>
#include <ngf/Graphics/Shader.h>
#include <ngf/Graphics/Texture.h>
#include <engge/Engine/Light.hpp>

namespace ng {
class LightingShader final : public ngf::Shader {
public:
  static constexpr int MaxLights = 50;

public:
  LightingShader();

  void setContentSize(glm::vec2 size);
  void setSpritePosInSheet(glm::vec2 spritePosInSheet);
  void setSpriteSizeRelToSheet(glm::vec2 spriteSizeRelToSheet);
  void setSpriteOffset(glm::vec2 spriteOffset);
  void setAmbientColor(ngf::Color color);
  [[nodiscard]] ngf::Color getAmbientColor() const;

  void setTexture(const ngf::Texture &texture);

  void setNumberLights(int numberLights);
  [[nodiscard]] int getNumberLights() const;

  void setLights(const std::array<Light, MaxLights> &lights);

private:
  int m_numberLights{0};
  ngf::Color m_ambient{ngf::Colors::White};
};
}
