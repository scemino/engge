#include <regex>
#include <ngf/System/Mouse.h>
#include <ngf/Graphics/Text.h>
#include <engge/Dialog/DialogManager.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/Text.hpp>

namespace ng {
namespace {
constexpr float DialogTop = 504.f;
const wchar_t *const Bullet = L"\u25CF ";
constexpr float SlidingSpeed = 25.f;

ngf::frect getGlobalBounds(const ngf::Text &text) {
  return ngf::transform(text.getTransform().getTransform(), text.getLocalBounds());
}
}

void DialogManager::setEngine(Engine *pEngine) {
  m_pEngine = pEngine;
  m_pEngineDialogScript = std::make_unique<EngineDialogScript>(*pEngine);
  m_pPlayer = std::make_unique<DialogPlayer>(*m_pEngineDialogScript);
}

void DialogManager::start(const std::string &actor, const std::string &name, const std::string &node) {
  m_pPlayer->start(actor, name, node);

  auto oldState = m_state;
  m_state = m_pPlayer->getState();

  if (oldState != m_state) {
    if (m_state == DialogManagerState::WaitingForChoice) {
      updateDialogSlots();
    } else if (m_state == DialogManagerState::None) {
      onDialogEnded();
    }
  }
}

void DialogManager::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (m_state != DialogManagerState::WaitingForChoice)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts = m_pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                  PreferenceDefaultValues::RetroFonts);
  const GGFont &font = m_pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = DialogTop;

  auto actorName = m_pPlayer->getActor();
  auto dialogHighlight = m_pEngine->getVerbUiColors(actorName)->dialogHighlight;
  auto dialogNormal = m_pEngine->getVerbUiColors(actorName)->dialogNormal;

  ng::Text text;
  text.setFont(font);
  auto hoverDone = false;
  for (const auto &slot : m_slots) {
    if (!slot.pChoice)
      continue;

    std::wstring s;
    s = Bullet;
    s += slot.text;
    text.setWideString(s);
    text.getTransform().setPosition({slot.pos.x, y + slot.pos.y});
    auto bounds = getGlobalBounds(text);
    auto hover = bounds.contains(m_mousePos);
    text.setColor(hover && !hoverDone ? dialogHighlight : dialogNormal);
    hoverDone |= hover;
    text.draw(target, {});

    y += (2.f * getGlobalBounds(text).getHeight() / 3.f);
  }

  target.setView(view);
}

void DialogManager::update(const ngf::TimeSpan &elapsed) {
  m_pPlayer->update();
  auto oldState = m_state;
  m_state = m_pPlayer->getState();

  if (oldState != m_state && m_state == DialogManagerState::WaitingForChoice) {
    updateDialogSlots();
  }

  updateChoices(elapsed);
}

void DialogManager::updateDialogSlots() {
  int i = 0;
  for (const auto &pStatement : m_pPlayer->getChoices()) {
    if (pStatement) {
      auto pChoice = dynamic_cast<Ast::Choice *>(pStatement->expression.get());
      auto text = pChoice->text;
      if (!text.empty() && text[0] == '$') {
        text = m_pEngine->executeDollar(text.substr(1));
      }
      std::wstring dialogText = ng::Engine::getText(text);
      std::wregex re(L"(\\{([^\\}]*)\\})");
      std::wsmatch matches;
      if (std::regex_search(dialogText, matches, re)) {
        dialogText = matches.suffix();
      }
      m_slots[i].text = dialogText;
      m_slots[i].pos = {0, 0};
    }
    m_slots[i].pChoice = pStatement;
    i++;
  }
}

void DialogManager::updateChoices(const ngf::TimeSpan &elapsed) {
  if (m_state != DialogManagerState::WaitingForChoice)
    return;

  auto retroFonts = m_pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                  PreferenceDefaultValues::RetroFonts);
  const GGFont &font = m_pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = DialogTop;
  int dialog = 0;
  for (const auto &dlg : m_slots) {
    if (dlg.pChoice == nullptr)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    std::wstring s;
    s = Bullet;
    s += dlg.text;
    ng::Text text;
    text.setFont(font);
    text.getTransform().setPosition({dlg.pos.x, dlg.pos.y + y});
    text.setWideString(s);
    auto bounds = getGlobalBounds(text);
    if (bounds.getWidth() > Screen::Width) {
      if (bounds.contains(m_mousePos)) {
        if ((bounds.getWidth() + dlg.pos.x) > Screen::Width) {
          dlg.pos.x -= SlidingSpeed * elapsed.getTotalSeconds();
          if ((bounds.getWidth() + dlg.pos.x) < Screen::Width) {
            dlg.pos.x = Screen::Width - bounds.getWidth();
          }
        }
      } else {
        if (dlg.pos.x < 0) {
          dlg.pos.x += SlidingSpeed * elapsed.getTotalSeconds();
          if (dlg.pos.x > 0) {
            dlg.pos.x = 0;
          }
        }
      }
    }
    y += bounds.getHeight() / 2.f;
    dialog++;
  }

  if (!ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left))
    return;

  y = DialogTop;
  dialog = 0;

  for (const auto &slot : m_slots) {
    if (!slot.pChoice)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    std::wstring s;
    s = Bullet;
    s += slot.text;
    ng::Text text;
    text.setFont(font);
    text.getTransform().setPosition({slot.pos.x, slot.pos.y + y});
    text.setWideString(s);
    if (getGlobalBounds(text).contains(m_mousePos)) {
      choose(dialog + 1);
      break;
    }
    y += getGlobalBounds(text).getHeight() / 2.f;
    dialog++;
  }
}

void DialogManager::choose(int choice) {
  if ((choice < 1) || (choice > static_cast<int>(m_slots.size())))
    return;

  ScriptEngine::rawCall("onChoiceClick");
  m_pPlayer->choose(choice);
}

void DialogManager::setMousePosition(glm::vec2 pos) {
  m_mousePos = pos;
}

void DialogManager::onDialogEnded() {
  ScriptEngine::call("onDialogEnded");
}

} // namespace ng
