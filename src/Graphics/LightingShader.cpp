#include <engge/Graphics/LightingShader.h>

namespace ng {
namespace {
constexpr const char *vertexShaderCode =
    R"(#version 100
precision mediump float;
attribute vec2 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoords;

uniform mat3 u_transform;

varying vec4 v_color;
varying vec2 v_texCoords;

void main(void) {
  v_color = a_color;
  v_texCoords = a_texCoords;
  vec3 worldPosition = vec3(a_position, 1);
  vec3 normalizedPosition = worldPosition * u_transform;
  gl_Position = vec4(normalizedPosition.xy, 0, 1);
})";

constexpr const char *fragmentShaderCode = R"(#version 100
#ifdef GL_ES
precision highp float;
#endif

varying vec2 v_texCoords;
varying vec4 v_color;

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
    vec4 texColor = texture2D(u_texture, v_texCoords);

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

            diffuse += ltdiffuse*coneFalloff;;
        }
    }
    vec4 finalCol = texColor * v_color;
    vec3 finalLight = (diffuse+u_ambientColor);
    finalLight = min( finalLight, vec3(1,1,1) );
    gl_FragColor = vec4(finalCol.rgb*finalLight, finalCol.a);
})";
}

LightingShader::LightingShader() {
  load(vertexShaderCode, fragmentShaderCode);
}

void LightingShader::setContentSize(glm::vec2 size) {
  setUniform("u_contentSize", size);
}

void LightingShader::setSpritePosInSheet(glm::vec2 spritePosInSheet) {
  setUniform("u_spritePosInSheet", spritePosInSheet);
}

void LightingShader::setSpriteSizeRelToSheet(glm::vec2 spriteSizeRelToSheet) {
  setUniform("u_spriteSizeRelToSheet", spriteSizeRelToSheet);
}

void LightingShader::setSpriteOffset(glm::vec2 spriteOffset) {
  setUniform("u_spriteOffset", spriteOffset);
}

void LightingShader::setAmbientColor(ngf::Color color) {
  setUniform3("u_ambientColor", color);
  m_ambient = color;
}

ngf::Color LightingShader::getAmbientColor() const { return m_ambient; }

void LightingShader::setTexture(const ngf::Texture &texture) {
  setUniform("u_texture", texture);
}

void LightingShader::setNumberLights(int numberLights) {
  setUniform("u_numberLights", numberLights);
  m_numberLights = std::min(numberLights, LightingShader::MaxLights);
}

int LightingShader::getNumberLights() const { return m_numberLights; }

void LightingShader::setLights(const std::array<Light, MaxLights> &lights) {
  std::array<glm::vec3, MaxLights> u_lightPos{};
  std::array<glm::vec2, MaxLights> u_coneDirection{};
  std::array<float, MaxLights> u_coneCosineHalfConeAngle{};
  std::array<float, MaxLights> u_coneFalloff{};
  std::array<ngf::Color, MaxLights> u_lightColor;
  std::array<float, MaxLights> u_brightness{};
  std::array<float, MaxLights> u_cutoffRadius{};
  std::array<float, MaxLights> u_halfRadius{};

  int numLights = 0;
  for (int i = 0; i < m_numberLights; ++i) {
    auto &light = lights[i];
    if (!light.on)
      continue;
    auto direction = light.coneDirection - 90.f;
    u_coneDirection[numLights] = glm::vec2(std::cos(glm::radians(direction)), std::sin(glm::radians(direction)));
    u_coneCosineHalfConeAngle[numLights] = cos(glm::radians(light.coneAngle / 2.f));
    u_coneFalloff[numLights] = light.coneFalloff;
    u_lightColor[numLights] = light.color;
    u_lightPos[numLights] = glm::vec3(light.pos, 1.f);
    u_brightness[numLights] = light.brightness;
    u_cutoffRadius[numLights] = std::max(1.0f, light.cutOffRadius);
    u_halfRadius[numLights] = std::max(0.01f, std::min(0.99f, light.halfRadius));
    numLights++;
  }
  m_numberLights = numLights;
  setUniformArray("u_lightPos", u_lightPos.data(), MaxLights);
  setUniformArray("u_coneDirection", u_coneDirection.data(), MaxLights);
  setUniformArray("u_coneCosineHalfConeAngle", u_coneCosineHalfConeAngle.data(), MaxLights);
  setUniformArray("u_coneFalloff", u_coneFalloff.data(), MaxLights);
  setUniformArray3("u_lightColor", u_lightColor.data(), MaxLights);
  setUniformArray("u_brightness", u_brightness.data(), MaxLights);
  setUniformArray("u_cutoffRadius", u_cutoffRadius.data(), MaxLights);
  setUniformArray("u_halfRadius", u_halfRadius.data(), MaxLights);
}
}
