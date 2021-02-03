#pragma once
#include <glm/vec3.hpp>

namespace ng {
struct RoomEffect {
  float iGlobalTime{0.f};
  float iFade{1.f};
  float wobbleIntensity{1.f};
  glm::vec3 shadows{-0.3f, 0, 0};
  glm::vec3 midtones{-0.2f, 0, 0.1f};
  glm::vec3 highlights{0, 0, 0.2f};

  float sepiaFlicker{1.f};
  std::array<float,5> RandomValue;
  float TimeLapse{0.f};

  float iNoiseThreshold{1.f};

  void reset() {
    iFade = 1.f;
    wobbleIntensity = 1.f;
    shadows = {-0.3f, 0, 0};
    midtones = {-0.2f, 0, 0.1f};
    highlights = {0, 0, 0.2f};
  }
};
}