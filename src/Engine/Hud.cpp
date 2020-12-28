#include <ngf/Graphics/Sprite.h>
#include "engge/Engine/Hud.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/System/Locator.hpp"

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
  glm::vec2 size(Screen::Width / 6.f, Screen::Height / 14.f);
  for (int i = 0; i < 9; i++) {
    auto left = (i / 3) * size.x;
    auto top = Screen::Height - size.y * 3 + (i % 3) * size.y;
    _verbRects.at(i) = ngf::irect::fromPositionSize({left, top}, {size.x, size.y});
  }

  // TODO: load verb shader
  //_verbShader.load(_verbShaderCode, nullptr);
  // TODO:
  //_verbShader.setUniform("colorMap", sf::Shader::CurrentTexture);
}

bool Hud::isMouseOver() const {
  if (!_active)
    return false;
  return _mousePos.y >= _verbRects.at(0).getTopLeft().y;
}

void Hud::setTextureManager(ResourceManager *pTextureManager) {
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

void Hud::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (!_isVisible)
    return;
  if (_currentActorIndex == -1 || getVerbSlot(_currentActorIndex).getVerb(0).id == 0)
    return;

  auto pVerb = _pVerbOverride;
  if (!pVerb) {
    pVerb = _pVerb;
  }
  auto verbId = pVerb->id;
  if (_pHoveredEntity && verbId == VerbConstants::VERB_WALKTO) {
    verbId = _pHoveredEntity->getDefaultVerb(VerbConstants::VERB_LOOKAT);
  } else {
    for (int i = 0; i < static_cast<int>(_verbRects.size()); i++) {
      if (_verbRects.at(i).contains((glm::ivec2) _mousePos)) {
        verbId = _verbSlots.at(_currentActorIndex).getVerb(1 + i).id;
        break;
      }
    }
  }

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  // draw UI background
  const auto &preferences = Locator<Preferences>::get();
  auto hudSentence = preferences.getUserPreference(PreferenceNames::HudSentence, PreferenceDefaultValues::HudSentence);
  auto uiBackingAlpha =
      preferences.getUserPreference(PreferenceNames::UiBackingAlpha, PreferenceDefaultValues::UiBackingAlpha);
  auto invertVerbHighlight = preferences.getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                           PreferenceDefaultValues::InvertVerbHighlight);
  const auto &verbUiColors = getVerbUiColors(_currentActorIndex);
  auto verbHighlight = invertVerbHighlight ? ngf::Colors::White : verbUiColors.verbHighlight;
  auto verbColor = invertVerbHighlight ? verbUiColors.verbHighlight : ngf::Colors::White;
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  auto uiBackingRect = hudSentence ? gameSheet.getRect("ui_backing_tall") : gameSheet.getRect("ui_backing");
  ngf::Sprite uiBacking;
  uiBacking.setColor(ngf::Color(0.f, 0.f, 0.f, uiBackingAlpha * _alpha));
  uiBacking.getTransform().setPosition({0, 720.f - uiBackingRect.getHeight()});
  uiBacking.setTexture(gameSheet.getTexture());
  uiBacking.setTextureRect(uiBackingRect);
  uiBacking.draw(target, {});

  // TODO: draw verbs
//  _verbShader.setUniform("ranges", glm::vec2(0.8f, 0.8f));
//  _verbShader.setUniform4("shadowColor", verbUiColors.verbNormalTint);
//  _verbShader.setUniform4("normalColor", verbUiColors.verbHighlight);
//  _verbShader.setUniform4("highlightColor", verbUiColors.verbHighlightTint);

  ngf::RenderStates verbStates;
  //TODO: verbStates.shader = &_verbShader;
  auto &verbSheet = Locator<ResourceManager>::get().getSpriteSheet("VerbSheet");
  for (int i = 1; i <= 9; i++) {
    auto verb = getVerbSlot(_currentActorIndex).getVerb(i);
    auto color = verb.id == verbId ? verbHighlight : verbColor;
    color.a = _alpha;
    //_verbShader.setUniform4("color", color);

    auto verbName = getVerbName(verb);
    auto rect = verbSheet.getRect(verbName);
    auto s = verbSheet.getSpriteSourceSize(verbName);
    ngf::Sprite verbSprite;
    verbSprite.setColor(color);
    verbSprite.getTransform().setPosition(s.getTopLeft());
    verbSprite.setTexture(verbSheet.getTexture());
    verbSprite.setTextureRect(rect);
    verbSprite.draw(target, verbStates);
  }

  target.setView(view);

  _inventory.draw(target, {});
}

void Hud::setCurrentActorIndex(int index) {
  _currentActorIndex = index;
  _inventory.setCurrentActorIndex(index);
  _inventory.setVerbUiColors(&getVerbUiColors(_currentActorIndex));
}

void Hud::setCurrentActor(Actor *pActor) {
  _inventory.setCurrentActor(pActor);
}

glm::vec2 Hud::findScreenPosition(int verbId) const {
  auto pVerb = getVerb(verbId);
  auto s = getVerbName(*pVerb);
  auto &verbSheet = Locator<ResourceManager>::get().getSpriteSheet("VerbSheet");
  auto r = verbSheet.getSpriteSourceSize(s);
  return glm::vec2(r.getTopLeft().x + r.getWidth() / 2.f, Screen::Height - (r.getTopLeft().y + r.getHeight() / 2.f));
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

void Hud::setMousePosition(glm::vec2 pos) {
  _mousePos = pos;
  _inventory.setMousePosition(pos);
}

const Verb *Hud::getHoveredVerb() const {
  if (_currentActorIndex == -1)
    return nullptr;

  for (int i = 0; i < static_cast<int>(_verbRects.size()); i++) {
    if (_verbRects.at(i).contains((glm::ivec2) _mousePos)) {
      auto verbId = getVerbSlot(_currentActorIndex).getVerb(1 + i).id;
      return getVerb(verbId);
    }
  }
  return nullptr;
}

void Hud::update(const ngf::TimeSpan &elapsed) {
  if (_state == State::FadeIn) {
    _alpha += elapsed.getTotalSeconds();
    if (_alpha >= 1.f) {
      _state = State::On;
      _alpha = 1.f;
    }
  } else if (_state == State::FadeOut) {
    _alpha -= elapsed.getTotalSeconds();
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