#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/View.hpp>
#include "Engine/Hud.hpp"
#include "Engine/Preferences.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "System/Locator.hpp"

namespace ng {
static const char *_verbShaderCode = "\n"
                                     "#ifdef GL_ES\n"
                                     "precision highp float;\n"
                                     "#endif\n"
                                     "\n"
                                     "\n"
                                     "uniform vec4 color;\n"
                                     "uniform vec4 shadowColor;\n"
                                     "uniform vec4 normalColor;\n"
                                     "uniform vec4 highlightColor;\n"
                                     "uniform vec2 ranges;\n"
                                     "uniform sampler2D colorMap;\n"
                                     "\n"
                                     "void main(void)\n"
                                     "{\n"
                                     "    float shadows = ranges.x;\n"
                                     "    float highlights = ranges.y;\n"
                                     "    \n"
                                     "    vec4 texColor = texture2D( colorMap, gl_TexCoord[0].xy);\n"
                                     "    \n"
                                     "    if ( texColor.g <= shadows)\n"
                                     "    {\n"
                                     "        texColor*=shadowColor;\n"
                                     "    }\n"
                                     "    else if (texColor.g >= highlights)\n"
                                     "    {\n"
                                     "        texColor*=highlightColor;\n"
                                     "    }\n"
                                     "    else\n"
                                     "    {\n"
                                     "        texColor*=normalColor;\n"
                                     "    }\n"
                                     "    texColor *= color;\n"
                                     "    gl_FragColor = texColor;\n"
                                     "}\n";

Hud::Hud() {
  sf::Vector2f size(Screen::Width / 6.f, Screen::Height / 14.f);
  for (int i = 0; i < 9; i++) {
    auto left = (i / 3) * size.x;
    auto top = Screen::Height - size.y * 3 + (i % 3) * size.y;
    _verbRects.at(i) = sf::IntRect(left, top, size.x, size.y);
  }

  // load verb shader
  if (!_verbShader.loadFromMemory(_verbShaderCode, sf::Shader::Type::Fragment)) {
    std::cerr << "Error loading shaders" << std::endl;
    return;
  }
  _verbShader.setUniform("colorMap", sf::Shader::CurrentTexture);
}

bool Hud::isMouseOver() const {
  if(!_active) return false;
  return _mousePos.y >= _verbRects.at(0).top;
}

void Hud::setTextureManager(TextureManager *pTextureManager) {
  _inventory.setTextureManager(pTextureManager);
}

void Hud::setVerb(int characterSlot, int verbSlot, const Verb &verb) {
  _verbSlots.at(characterSlot).setVerb(verbSlot, verb);
}

[[nodiscard]] const VerbSlot &Hud::getVerbSlot(int characterSlot) const {
  return _verbSlots.at(characterSlot);
}

void Hud::setVerbUiColors(int characterSlot, VerbUiColors colors) {
  _verbUiColors.at(characterSlot) = colors;
}

[[nodiscard]] const VerbUiColors &Hud::getVerbUiColors(int characterSlot) const {
  return _verbUiColors.at(characterSlot);
}

void Hud::draw(sf::RenderTarget &target, sf::RenderStates) const {
  if(!_isVisible) return;
  if (_currentActorIndex == -1 || getVerbSlot(_currentActorIndex).getVerb(0).id == 0)
    return;

  auto pVerb = _pVerbOverride;
  if (!pVerb) {
    pVerb = _pVerb;
  }
  auto verbId = pVerb->id;
  if (_pHoveredEntity && verbId == VerbConstants::VERB_WALKTO) {
    verbId = getDefaultVerb(_pHoveredEntity);
  } else {
    for (int i = 0; i < static_cast<int>(_verbRects.size()); i++) {
      if (_verbRects.at(i).contains((sf::Vector2i) _mousePos)) {
        verbId = _verbSlots.at(_currentActorIndex).getVerb(1 + i).id;
        break;
      }
    }
  }

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

  // draw UI background
  const auto& preferences = Locator<Preferences>::get();
  auto hudSentence = preferences.getUserPreference(PreferenceNames::HudSentence, PreferenceDefaultValues::HudSentence);
  auto uiBackingAlpha =
      preferences.getUserPreference(PreferenceNames::UiBackingAlpha, PreferenceDefaultValues::UiBackingAlpha);
  auto invertVerbHighlight = preferences.getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                           PreferenceDefaultValues::InvertVerbHighlight);
  const auto &verbUiColors = getVerbUiColors(_currentActorIndex);
  auto verbHighlight = invertVerbHighlight ? sf::Color::White : verbUiColors.verbHighlight;
  auto verbColor = invertVerbHighlight ? verbUiColors.verbHighlight : sf::Color::White;
  auto &gameSheet = Locator<TextureManager>::get().getSpriteSheet("GameSheet");
  auto uiBackingRect = hudSentence ? gameSheet.getRect("ui_backing_tall") : gameSheet.getRect("ui_backing");
  sf::Sprite uiBacking;
  uiBacking.setColor(sf::Color(0, 0, 0, uiBackingAlpha * _alpha * 255));
  uiBacking.setPosition(0, 720.f - uiBackingRect.height);
  uiBacking.setTexture(gameSheet.getTexture());
  uiBacking.setTextureRect(uiBackingRect);
  target.draw(uiBacking);

  // draw verbs
  _verbShader.setUniform("ranges", sf::Vector2f(0.8f, 0.8f));
  _verbShader.setUniform("shadowColor", sf::Glsl::Vec4(verbUiColors.verbNormalTint));
  _verbShader.setUniform("normalColor", sf::Glsl::Vec4(verbUiColors.verbHighlight));
  _verbShader.setUniform("highlightColor", sf::Glsl::Vec4(verbUiColors.verbHighlightTint));

  sf::RenderStates verbStates;
  verbStates.shader = &_verbShader;
  auto &verbSheet = Locator<TextureManager>::get().getSpriteSheet("VerbSheet");
  for (int i = 1; i <= 9; i++) {
    auto verb = getVerbSlot(_currentActorIndex).getVerb(i);
    auto color = verb.id == verbId ? verbHighlight : verbColor;
    color.a = static_cast<sf::Uint8>(_alpha * 255.f);
    _verbShader.setUniform("color", sf::Glsl::Vec4(color));

    auto verbName = getVerbName(verb);
    auto rect = verbSheet.getRect(verbName);
    auto s = verbSheet.getSpriteSourceSize(verbName);
    sf::Sprite verbSprite;
    verbSprite.setColor(color);
    verbSprite.setPosition(s.left, s.top);
    verbSprite.setTexture(verbSheet.getTexture());
    verbSprite.setTextureRect(rect);
    target.draw(verbSprite, verbStates);
  }

  target.setView(view);

  target.draw(_inventory);
}

void Hud::setCurrentActorIndex(int index) {
  _currentActorIndex = index;
  _inventory.setCurrentActorIndex(index);
  _inventory.setVerbUiColors(&getVerbUiColors(_currentActorIndex));
}

void Hud::setCurrentActor(Actor *pActor) {
  _inventory.setCurrentActor(pActor);
}

sf::Vector2f Hud::findScreenPosition(int verbId) const {
  auto pVerb = getVerb(verbId);
  auto s = getVerbName(*pVerb);
  auto &verbSheet = Locator<TextureManager>::get().getSpriteSheet("VerbSheet");
  auto r = verbSheet.getSpriteSourceSize(s);
  return sf::Vector2f(r.left + r.width / 2.f, Screen::Height - (r.top + r.height / 2.f));
}

const Verb *Hud::getVerb(int id) const {
  auto index = _currentActorIndex;
  if (index < 0)
    return nullptr;
  const auto &verbSlot = getVerbSlot(index);
  for (auto i = 0; i < 10; i++) {
    const auto &verb = verbSlot.getVerb(i);
    if (verb.id == id) {
      return &verb;
    }
  }
  return nullptr;
}

std::string Hud::getVerbName(const Verb &verb) {
  const auto &preferences = Locator<Preferences>::get();
  auto lang = preferences.getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
  auto isRetro = preferences.getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs);
  std::string s;
  s.append(verb.image).append(isRetro ? "_retro" : "").append("_").append(lang);
  return s;
}

int Hud::getDefaultVerb(Entity *pEntity) {
  const char *dialog = nullptr;
  if (ScriptEngine::rawGet(pEntity, "dialog", dialog) && dialog)
    return VerbConstants::VERB_TALKTO;

  int value = 0;
  if (ScriptEngine::rawGet(pEntity, "defaultVerb", value))
    return value;

  return VerbConstants::VERB_LOOKAT;
}

void Hud::setMousePosition(sf::Vector2f pos) {
  _mousePos = pos;
  _inventory.setMousePosition(pos);
}

const Verb *Hud::getHoveredVerb() const {
  if (_currentActorIndex == -1)
    return nullptr;

  for (int i = 0; i < static_cast<int>(_verbRects.size()); i++) {
    if (_verbRects.at(i).contains((sf::Vector2i) _mousePos)) {
      auto verbId = getVerbSlot(_currentActorIndex).getVerb(1 + i).id;
      return getVerb(verbId);
    }
  }
  return nullptr;
}

void Hud::update(const sf::Time &elapsed) {
  if (_state == State::FadeIn) {
    _alpha += elapsed.asSeconds();
    if (_alpha >= 1.f) {
      _state = State::On;
      _alpha = 1.f;
    }
  } else if (_state == State::FadeOut) {
    _alpha -= elapsed.asSeconds();
    if (_alpha <= 0.f) {
      _state = State::Off;
      _alpha = 0.f;
    }
  }
  _inventory.setAlpha(_alpha);
  _inventory.update(elapsed);
}

void Hud::setActive(bool active) {
  if (!_active && active) {
    _state = State::FadeIn;
  } else if (_active && !active) {
    _state = State::FadeOut;
  }
  _active = active;
}

}