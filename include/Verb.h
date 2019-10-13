#pragma once
#include "SFML/Graphics/Color.hpp"
#include <array>
#include <string>

namespace ng
{
namespace VerbConstants
{
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

struct Verb
{
    int id;
    std::string image;
    std::string func;
    std::string text;
    std::string key;

    static std::string getName(int id)
    {
        switch (id)
        {
            case VerbConstants::VERB_CLOSE:
                return "verbClose";
            case VerbConstants::VERB_GIVE:
                return "verbGive";
            case VerbConstants::VERB_LOOKAT:
                return "verbLookAt";
            case VerbConstants::VERB_OPEN:
                return "verbOpen";
            case VerbConstants::VERB_PICKUP:
                return "verbPickup";
            case VerbConstants::VERB_PULL:
                return "verbPull";
            case VerbConstants::VERB_PUSH:
                return "verbPush";
            case VerbConstants::VERB_WALKTO:
                return "verbTalkTo";
            case VerbConstants::VERB_USE:
                return "verbUse";
        }
        return "";
    }
};

class VerbSlot
{
  public:
    void setVerb(int index, const Verb &verb) { _verbs.at(index) = verb; }
    const Verb &getVerb(int index) const { return _verbs.at(index); }
    size_t getVerbIndex(int id) const
    {
        for (size_t i = 0; i < _verbs.size(); i++)
        {
            if (_verbs.at(i).id == id)
                return i;
        }
        return -1;
    }

  private:
    std::array<Verb, 10> _verbs;
};

struct VerbUiColors
{
    sf::Color sentence;
    sf::Color verbNormal;
    sf::Color verbNormalTint;
    sf::Color verbHighlight;
    sf::Color verbHighlightTint;
    sf::Color dialogNormal;
    sf::Color dialogHighlight;
    sf::Color inventoryFrame;
    sf::Color inventoryBackground;
    sf::Color retroNormal;
    sf::Color retroHighlight;
};
} // namespace ng