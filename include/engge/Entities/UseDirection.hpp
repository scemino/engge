#pragma once
#include <engge/Entities/DirectionConstants.hpp>

namespace ng {
enum class UseDirection {
  Front = DirectionConstants::FACE_FRONT,
  Back = DirectionConstants::FACE_BACK,
  Left = DirectionConstants::FACE_LEFT,
  Right = DirectionConstants::FACE_RIGHT,
};
}
