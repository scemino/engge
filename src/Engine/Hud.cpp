#include <ngf/Graphics/Sprite.h>
#include "engge/Engine/Hud.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/System/Locator.hpp"
#include "Shaders.hpp"

namespace ng {
Hud::Hud() {
  glm::vec2 size(Screen::Width / 6.f, Screen::Height / 14.f);
  for (int i = 0; i < 9; i++) {
    auto left = static_cast<float>(i / 3) * size.x;
    auto top = Screen::Height - size.y * 3 + static_cast<float>(i % 3) * size.y;
    m_verbRects.at(i) = ngf::irect::fromPositionSize({left, top}, {size.x, size.y});
  }

  m_verbShader.load(Shaders::verbVertexShaderCode, Shaders::verbFragmentShaderCode);
}

Hud::~Hud() = default;

bool Hud::isMouseOver() const {
  if (!m_active)
    return false;
  return m_mousePos.y >= m_verbRects.at(0).getTopLeft().y;
}

void Hud::setTextureManager(ResourceManager *pTextureManager) {
  m_inventory.setTextureManager(pTextureManager);
}

void Hud::setVerb(int characterSlot, int verbSlot, const Verb &verb) {
  m_verbSlots.at(characterSlot).setVerb(verbSlot, verb);
}

[[nodiscard]] const VerbSlot &Hud::getVerbSlot(int characterSlot) const {
  return m_verbSlots.at(characterSlot);
}

void Hud::setVerbUiColors(int characterSlot, VerbUiColors colors) {
  m_verbUiColors.at(characterSlot) = colors;
}

[[nodiscard]] const VerbUiColors &Hud::getVerbUiColors(int characterSlot) const {
  return m_verbUiColors.at(characterSlot);
}

void Hud::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (!m_isVisible)
    return;
  if (m_currentActorIndex == -1 || getVerbSlot(m_currentActorIndex).getVerb(0).id == 0)
    return;

  auto pVerb = m_pVerbOverride;
  if (!pVerb) {
    pVerb = m_pVerb;
  }
  auto verbId = pVerb->id;
  if (m_pHoveredEntity && verbId == VerbConstants::VERB_WALKTO) {
    verbId = m_pHoveredEntity->getDefaultVerb(VerbConstants::VERB_LOOKAT);
  } else {
    for (int i = 0; i < static_cast<int>(m_verbRects.size()); i++) {
      if (m_verbRects.at(i).contains((glm::ivec2) m_mousePos)) {
        verbId = m_verbSlots.at(m_currentActorIndex).getVerb(1 + i).id;
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
  const auto &verbUiColors = getVerbUiColors(m_currentActorIndex);
  auto verbHighlight = invertVerbHighlight ? ngf::Colors::White : verbUiColors.verbHighlight;
  auto verbColor = invertVerbHighlight ? verbUiColors.verbHighlight : ngf::Colors::White;
  auto &gameSheet = Locator<ResourceManager>::get().getSpriteSheet("GameSheet");
  auto uiBackingRect = hudSentence ? gameSheet.getRect("ui_backing_tall") : gameSheet.getRect("ui_backing");

  ngf::Sprite uiBacking(*gameSheet.getTexture(), uiBackingRect);
  uiBacking.setColor(ngf::Color(0.f, 0.f, 0.f, uiBackingAlpha * m_alpha));
  uiBacking.getTransform().setPosition({0, 720.f - uiBackingRect.getHeight()});
  uiBacking.draw(target, {});

  m_verbShader.setUniform("u_ranges", glm::vec2(0.8f, 0.8f));
  m_verbShader.setUniform4("u_shadowColor", verbUiColors.verbNormalTint);
  m_verbShader.setUniform4("u_normalColor", verbUiColors.verbHighlight);
  m_verbShader.setUniform4("u_highlightColor", verbUiColors.verbHighlightTint);

  ngf::RenderStates verbStates;
  verbStates.shader = &m_verbShader;
  auto &verbSheet = Locator<ResourceManager>::get().getSpriteSheet("VerbSheet");
  for (int i = 1; i <= 9; i++) {
    auto verb = getVerbSlot(m_currentActorIndex).getVerb(i);
    auto color = verb.id == verbId ? verbHighlight : verbColor;
    color.a = m_alpha;

    auto verbName = getVerbName(verb);
    auto rect = verbSheet.getRect(verbName);
    auto s = verbSheet.getSpriteSourceSize(verbName);
    ngf::Sprite verbSprite(*verbSheet.getTexture(), rect);
    verbSprite.setColor(color);
    verbSprite.getTransform().setPosition(s.getTopLeft());
    verbSprite.draw(target, verbStates);
  }

  target.setView(view);

  m_inventory.draw(target, {});
}

void Hud::setCurrentActorIndex(int index) {
  m_currentActorIndex = index;
  m_inventory.setCurrentActorIndex(index);
  m_inventory.setVerbUiColors(&getVerbUiColors(m_currentActorIndex));
}

void Hud::setCurrentActor(Actor *pActor) {
  m_inventory.setCurrentActor(pActor);
}

glm::vec2 Hud::findScreenPosition(int verbId) const {
  auto pVerb = getVerb(verbId);
  auto s = getVerbName(*pVerb);
  auto &verbSheet = Locator<ResourceManager>::get().getSpriteSheet("VerbSheet");
  auto r = verbSheet.getSpriteSourceSize(s);
  return glm::vec2(r.getTopLeft().x + r.getWidth() / 2.f, Screen::Height - (r.getTopLeft().y + r.getHeight() / 2.f));
}

const Verb *Hud::getVerb(int id) const {
  auto index = m_currentActorIndex;
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
  m_mousePos = pos;
  m_inventory.setMousePosition(pos);
}

const Verb *Hud::getHoveredVerb() const {
  if (m_currentActorIndex == -1)
    return nullptr;

  for (int i = 0; i < static_cast<int>(m_verbRects.size()); i++) {
    if (m_verbRects.at(i).contains((glm::ivec2) m_mousePos)) {
      auto verbId = getVerbSlot(m_currentActorIndex).getVerb(1 + i).id;
      return getVerb(verbId);
    }
  }
  return nullptr;
}

void Hud::update(const ngf::TimeSpan &elapsed) {
  if (m_state == State::FadeIn) {
    m_alpha += elapsed.getTotalSeconds();
    if (m_alpha >= 1.f) {
      m_state = State::On;
      m_alpha = 1.f;
    }
  } else if (m_state == State::FadeOut) {
    m_alpha -= elapsed.getTotalSeconds();
    if (m_alpha <= 0.f) {
      m_state = State::Off;
      m_alpha = 0.f;
    }
  }
  m_inventory.setAlpha(m_alpha);
  m_inventory.update(elapsed);
}

void Hud::setActive(bool active) {
  if (!m_active && active) {
    m_state = State::FadeIn;
  } else if (m_active && !active) {
    m_state = State::FadeOut;
  }
  m_active = active;
}
}