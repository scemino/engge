#pragma once
#include <string>
#include "SFML/Graphics/Color.hpp"

namespace gg
{
struct Verb
{
    std::string id;
    std::string image;
    std::string func;
    std::string text;
    std::string key;
};

class VerbSlot
{
  public:
    void setVerb(int index, const Verb &verb) { _verbs[index] = verb; }
    const Verb &getVerb(int index) const { return _verbs[index]; }

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
};
} // namespace gg