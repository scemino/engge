#pragma once
#include <engge/Entities/DirectionConstants.hpp>

namespace ng {
enum class Facing {
  FACE_FRONT = DirectionConstants::FACE_FRONT,
  FACE_BACK = DirectionConstants::FACE_BACK,
  FACE_LEFT = DirectionConstants::FACE_LEFT,
  FACE_RIGHT = DirectionConstants::FACE_RIGHT
};
}