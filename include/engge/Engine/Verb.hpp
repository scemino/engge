#pragma once
#include <array>
#include <string>
#include <ngf/Graphics/Color.h>

namespace ng {
namespace VerbConstants {
static const int VERB_WALKTO = 1;
static const int VERB_LOOKAT = 2;
static const int VERB_TALKTO = 3;
static const int VERB_PICKUP = 4;
static const int VERB_OPEN = 5;
static const int VERB_CLOSE = 6;
static const int VERB_PUSH = 7;
static const int VERB_PULL = 8;
static const int VERB_GIVE = 9;
static const int VERB_USE = 10;
static const int VERB_DIALOG = 13;
} // namespace VerbConstants

struct Verb {
  int id;
  std::string image;
  std::string func;
  std::string text;
  std::string key;

  static std::string getName(int id) {
    switch (id) {
    case VerbConstants::VERB_CLOSE:return "verbClose";
    case VerbConstants::VERB_GIVE:return "verbGive";
    case VerbConstants::VERB_LOOKAT:return "verbLookAt";
    case VerbConstants::VERB_OPEN:return "verbOpen";
    case VerbConstants::VERB_PICKUP:return "verbPickup";
    case VerbConstants::VERB_PULL:return "verbPull";
    case VerbConstants::VERB_PUSH:return "verbPush";
    case VerbConstants::VERB_TALKTO:return "verbTalkTo";
    case VerbConstants::VERB_WALKTO:return "verbWalkTo";
    case VerbConstants::VERB_USE:return "verbUse";
    }
    return "";
  }
};

class VerbSlot {
public:
  void setVerb(int index, const Verb &verb) { m_verbs.at(index) = verb; }
  [[nodiscard]] const Verb &getVerb(int index) const { return m_verbs.at(index); }

private:
  std::array<Verb, 10> m_verbs;
};

struct VerbUiColors {
  ngf::Color sentence;
  ngf::Color verbNormal;
  ngf::Color verbNormalTint;
  ngf::Color verbHighlight;
  ngf::Color verbHighlightTint;
  ngf::Color dialogNormal;
  ngf::Color dialogHighlight;
  ngf::Color inventoryFrame;
  ngf::Color inventoryBackground;
  ngf::Color retroNormal;
  ngf::Color retroHighlight;
};
} // namespace ng