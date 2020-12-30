#pragma once
#include <array>
#include <ngf/Graphics/Color.h>
#include <ngf/Graphics/Shader.h>
#include <ngf/Graphics/Texture.h>
#include <engge/Engine/Light.hpp>

namespace ng {
constexpr const char *vertexShaderCode =
    R"(#version 330 core
precision mediump float;
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texCoords;

uniform mat3 u_transform;

out vec4 v_color;
out vec2 v_texCoords;

void main(void) {
  v_color = a_color;
  v_texCoords = a_texCoords;
  vec3 worldPosition = vec3(a_position, 1);
  vec3 normalizedPosition = worldPosition * u_transform;
  gl_Position = vec4(normalizedPosition.xy, 0, 1);
})";

constexpr const char *fragmentShaderCode = R"(#version 330 core
precision highp float;

in vec2 v_texCoords;
in vec4 v_color;
out vec4 FragColor;

uniform sampler2D u_texture;

uniform vec2  u_contentSize;
uniform vec3  u_ambientColor;

uniform vec2 u_spritePosInSheet;
uniform vec2 u_spriteSizeRelToSheet;
uniform vec2 u_spriteOffset;

uniform int  u_numberLights;
uniform vec3  u_lightPos[50];
uniform vec3  u_lightColor[50];
uniform float u_brightness[50];
uniform float u_cutoffRadius[50];
uniform float u_halfRadius[50];
uniform float u_coneCosineHalfConeAngle[50];
uniform float u_coneFalloff[50];
uniform vec2  u_coneDirection[50];

void main(void)
{
    vec4 texColor=texture(u_texture, v_texCoords);

    vec2 spriteTexCoord = (v_texCoords - u_spritePosInSheet) / u_spriteSizeRelToSheet; // [0..1]
    vec2 pixelPos = spriteTexCoord * u_contentSize + u_spriteOffset; // [0..origSize]
    vec2 curPixelPosInLocalSpace = vec2(pixelPos.x, -pixelPos.y);

    vec3 diffuse = vec3(0,0,0);
    for ( int i = 0; i < u_numberLights; i ++)
    {
        vec2 lightVec = curPixelPosInLocalSpace.xy - u_lightPos[i].xy;
        float coneValue = dot( normalize(-lightVec), u_coneDirection[i] );
        if ( coneValue >= u_coneCosineHalfConeAngle[i] )
        {
            float intercept = u_cutoffRadius[i] * u_halfRadius[i];
            float dx_1 = 0.5 / intercept;
            float dx_2 = 0.5 / (u_cutoffRadius[i] - intercept);
            float offset = 0.5 + intercept * dx_2;

            float lightDist = length(lightVec);
            float falloffTermNear = clamp((1.0 - lightDist * dx_1), 0.0, 1.0);
            float falloffTermFar  = clamp((offset - lightDist * dx_2), 0.0, 1.0);
            float falloffSelect = step(intercept, lightDist);
            float falloffTerm = (1.0 - falloffSelect) * falloffTermNear + falloffSelect * falloffTermFar;
            float spotLight = u_brightness[i] * falloffTerm;

            vec3 ltdiffuse = vec3(u_brightness[i] * falloffTerm) * u_lightColor[i];

            float coneRange = 1.0-u_coneCosineHalfConeAngle[i];
            float halfConeRange = coneRange * u_coneFalloff[i];
            float conePos   = 1.0-coneValue;
            float coneFalloff = 1.0;
            if ( conePos > halfConeRange )
            {
                coneFalloff = 1.0-((conePos-halfConeRange)/(coneRange-halfConeRange));
            }

            diffuse += ltdiffuse*coneFalloff;
        }
    }
    // Clamp "diffuse+ambientcolor" to 1
    vec3 finalLight = (diffuse);
    vec4 finalCol = texColor * v_color;
    finalCol.rgb = finalCol.rgb * u_ambientColor;
    FragColor = vec4(finalCol.rgb + diffuse, finalCol.a);
}
)";

class LightingShader : public ngf::Shader {
public:
  LightingShader() {
    load(vertexShaderCode, fragmentShaderCode);
  }

  void setContentSize(glm::vec2 size) {
    setUniform("u_contentSize", size);
  }

  void setSpritePosInSheet(glm::vec2 spritePosInSheet) {
    setUniform("u_spritePosInSheet", spritePosInSheet);
  }

  void setSpriteSizeRelToSheet(glm::vec2 spriteSizeRelToSheet) {
    setUniform("u_spriteSizeRelToSheet", spriteSizeRelToSheet);
  }

  void setSpriteOffset(glm::vec2 spriteOffset) {
    setUniform("u_spriteOffset", spriteOffset);
  }

  void setAmbientColor(ngf::Color color) {
    setUniform3("u_ambientColor", color);
  }

  void setTexture(const ngf::Texture &texture) {
    setUniform("u_texture", texture);
  }

  void setNumberLights(int numberLights) {
    setUniform("u_numberLights", numberLights);
    m_numberLights = numberLights;
  }

  void setLights(const std::array<Light, 50> &lights) {
    std::array<glm::vec3, 50> u_lightPos;
    std::array<glm::vec2, 50> u_coneDirection;
    std::array<float, 50> u_coneCosineHalfConeAngle;
    std::array<float, 50> u_coneFalloff;
    std::array<ngf::Color, 50> u_lightColor;
    std::array<float, 50> u_brightness;
    std::array<float, 50> u_cutoffRadius;
    std::array<float, 50> u_halfRadius;

    for (size_t i = 0; i < lights.size(); ++i) {
      auto &light = lights[i];
      auto direction = light.coneDirection - 90.f;
      u_coneDirection[i] = glm::vec2(std::cos(glm::radians(direction)), std::sin(glm::radians(direction)));
      u_coneCosineHalfConeAngle[i] = cos(glm::radians(light.coneAngle / 2.f));
      u_coneFalloff[i] = light.coneFalloff;
      u_lightColor[i] = light.color;
      u_lightPos[i] = glm::vec3(light.pos, 1.f);
      u_brightness[i] = light.brightness;
      u_cutoffRadius[i] = std::max(1.0f, light.cutOffRadius);
      u_halfRadius[i] = std::max(0.01f, std::min(0.99f, light.halfRadius));
    }
    setUniformArray("u_lightPos", u_lightPos.data(), 50);
    setUniformArray("u_coneDirection", u_coneDirection.data(), 50);
    setUniformArray("u_coneCosineHalfConeAngle", u_coneCosineHalfConeAngle.data(), 50);
    setUniformArray("u_coneFalloff", u_coneFalloff.data(), 50);
    setUniformArray3("u_lightColor", u_lightColor.data(), 50);
    setUniformArray("u_brightness", u_brightness.data(), 50);
    setUniformArray("u_cutoffRadius", u_cutoffRadius.data(), 50);
    setUniformArray("u_halfRadius", u_halfRadius.data(), 50);
  }

private:
  int m_numberLights{0};
};
}
