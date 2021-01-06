#include <ngf/Graphics/Sprite.h>
#include "engge/Engine/Hud.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/System/Locator.hpp"

namespace ng {
static const char *_vertexShaderSource =
    R"(#version 330 core
precision mediump float;
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_texCoords;

uniform vec4 u_shadowColor;
uniform vec4 u_normalColor;
uniform vec4 u_highlightColor;
uniform vec2 u_ranges;
uniform mat3 u_transform;

out vec4 v_color;
out vec2 v_texCoords;
out vec4 v_shadowColor;
out vec4 v_normalColor;
out vec4 v_highlightColor;
out vec2 v_ranges;

void main(void) {
  v_color = a_color;
  v_texCoords = a_texCoords;
  v_shadowColor = u_shadowColor;
  v_normalColor = u_normalColor;
  v_highlightColor = u_highlightColor;
  v_ranges = u_ranges;

  vec3 worldPosition = vec3(a_position, 1);
  vec3 normalizedPosition = worldPosition * u_transform;
  gl_Position = vec4(normalizedPosition.xy, 0, 1);
})";

static const char *_verbShaderCode =
    R"(#version 330 core
#ifdef GL_ES
precision highp float;
#endif

out vec4 FragColor;

in vec4 v_color;
in vec2 v_texCoords;
in vec4 v_shadowColor;
in vec4 v_normalColor;
in vec4 v_highlightColor;
in vec2 v_ranges;
uniform sampler2D u_texture;

void main(void)
{
    float shadows = v_ranges.x;
    float highlights = v_ranges.y;

    vec4 texColor = texture(u_texture, v_texCoords);

    if ( texColor.g <= shadows)
    {
        texColor*=v_shadowColor;
    }
    else if (texColor.g >= highlights)
    {
        texColor*=v_highlightColor;
    }
    else
    {
        texColor*=v_normalColor;
    }
    texColor *= v_color;
    FragColor = texColor;
}
)";

Hud::Hud() {
  glm::vec2 size(Screen::Width / 6.f, Screen::Height / 14.f);
  for (int i = 0; i < 9; i++) {
    auto left = (i / 3) * size.x;
    auto top = Screen::Height - size.y * 3 + static_cast<float>(i % 3) * size.y;
    _verbRects.at(i) = ngf::irect::fromPositionSize({left, top}, {size.x, size.y});
  }

  _verbShader.load(_vertexShaderSource, _verbShaderCode);
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

  ngf::Sprite uiBacking(gameSheet.getTexture(), uiBackingRect);
  uiBacking.setColor(ngf::Color(0.f, 0.f, 0.f, uiBackingAlpha * _alpha));
  uiBacking.getTransform().setPosition({0, 720.f - uiBackingRect.getHeight()});
  uiBacking.draw(target, {});

  _verbShader.setUniform("u_ranges", glm::vec2(0.8f, 0.8f));
  _verbShader.setUniform4("u_shadowColor", verbUiColors.verbNormalTint);
  _verbShader.setUniform4("u_normalColor", verbUiColors.verbHighlight);
  _verbShader.setUniform4("u_highlightColor", verbUiColors.verbHighlightTint);

  ngf::RenderStates verbStates;
  verbStates.shader = &_verbShader;
  auto &verbSheet = Locator<ResourceManager>::get().getSpriteSheet("VerbSheet");
  for (int i = 1; i <= 9; i++) {
    auto verb = getVerbSlot(_currentActorIndex).getVerb(i);
    auto color = verb.id == verbId ? verbHighlight : verbColor;
    color.a = _alpha;

    auto verbName = getVerbName(verb);
    auto rect = verbSheet.getRect(verbName);
    auto s = verbSheet.getSpriteSourceSize(verbName);
    ngf::Sprite verbSprite(verbSheet.getTexture(), rect);
    verbSprite.setColor(color);
    verbSprite.getTransform().setPosition(s.getTopLeft());
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