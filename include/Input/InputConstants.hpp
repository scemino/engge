#pragma once
#include <functional>

namespace ng {

enum class InputConstants {
  NONE = 0,
  // these codes corresponds to SDL key codes used in TWP
  KEY_UP = 0x40000052,
  KEY_RIGHT = 0x4000004F,
  KEY_DOWN = 0x40000051,
  KEY_LEFT = 0x40000050,
  KEY_PAD1 = 0x40000059,
  KEY_PAD2 = 0x4000005A,
  KEY_PAD3 = 0x4000005B,
  KEY_PAD4 = 0x4000005C,
  KEY_PAD5 = 0x4000005D,
  KEY_PAD6 = 0x4000005E,
  KEY_PAD7 = 0x4000005F,
  KEY_PAD8 = 0x40000056,
  KEY_PAD9 = 0x40000061,
  KEY_ESCAPE = 0x08,
  KEY_TAB = 0x09,
  KEY_RETURN = 0x0D,
  KEY_BACKSPACE = 0x1B,
  KEY_SPACE = 0X20,
  KEY_A = 0x61,
  KEY_B = 0x62,
  KEY_C = 0x63,
  KEY_D = 0x64,
  KEY_E = 0x65,
  KEY_F = 0x66,
  KEY_G = 0x67,
  KEY_H = 0x68,
  KEY_I = 0x69,
  KEY_J = 0x6A,
  KEY_K = 0x6B,
  KEY_L = 0x6C,
  KEY_M = 0x6D,
  KEY_N = 0x6E,
  KEY_O = 0x6F,
  KEY_P = 0x70,
  KEY_Q = 0x71,
  KEY_R = 0x72,
  KEY_S = 0x73,
  KEY_T = 0x74,
  KEY_U = 0x75,
  KEY_V = 0x76,
  KEY_W = 0x77,
  KEY_X = 0x78,
  KEY_Y = 0x79,
  KEY_Z = 0x7A,
  KEY_0 = 0x30,
  KEY_1 = 0x31,
  KEY_2 = 0x32,
  KEY_3 = 0x33,
  KEY_4 = 0x34,
  KEY_5 = 0x35,
  KEY_6 = 0x36,
  KEY_7 = 0x37,
  KEY_8 = 0x38,
  KEY_9 = 0x39,
  KEY_F1 = 0x4000003A,
  KEY_F2 = 0x4000003B,
  KEY_F3 = 0x4000003C,
  KEY_F4 = 0x4000003D,
  KEY_F5 = 0x4000003E,
  KEY_F6 = 0x4000003F,
  KEY_F7 = 0x40000040,
  KEY_F8 = 0x40000041,
  KEY_F9 = 0x40000042,
  KEY_F10 = 0x40000043,
  KEY_F11 = 0x40000044,
  KEY_F12 = 0x40000045,

  BUTTON_A = 0x3E8,
  BUTTON_B = 0x3E9,
  BUTTON_X = 0x3EA,
  BUTTON_Y = 0x3EB,
  BUTTON_START = 0x3EC,
  BUTTON_BACK = 0x3EC,
  BUTTON_MOUSE_LEFT = 0x3ED,
  BUTTON_MOUSE_RIGHT = 0x3EE,
};

enum class MetaKeys {
  None,
  Alt,
  Control,
  Shift,
  System
};

struct Input {
public:
  Input(MetaKeys metaKey, InputConstants input) {
    this->metaKey = metaKey;
    this->input = input;
  }

  Input(InputConstants input) {
    this->input = input;
  }

  bool operator==(const Input &other) const {
    return (metaKey == other.metaKey
        && input == other.input);
  }

  MetaKeys metaKey{MetaKeys::None};
  InputConstants input;
};

struct InputHash {
  std::size_t operator()(const ng::Input &k) const noexcept {
    return std::hash<int>()(static_cast<int>(k.metaKey))
        ^ std::hash<int>()(static_cast<int>(k.input));
  }
};

}
