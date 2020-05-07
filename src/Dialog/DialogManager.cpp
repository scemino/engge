#include <regex>
#include "Dialog/DialogManager.hpp"
#include "Engine/Engine.hpp"
#include "System/Logger.hpp"
#include "Engine/Preferences.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Graphics/Text.hpp"

#include "Graphics/Screen.hpp"

namespace ng {

static constexpr float SlidingSpeed = 25.f;

void DialogManager::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
  _pEngineDialogScript = std::make_unique<EngineDialogScript>(*pEngine);
  _pPlayer = std::make_unique<DialogPlayer>(*_pEngineDialogScript.get());
}

void DialogManager::start(const std::string &actor, const std::string &name, const std::string &node) {
  _pPlayer->start(actor, name, node);

  auto oldState = _state;
  _state = _pPlayer->getState();

  if (oldState != _state && _state == DialogManagerState::WaitingForChoice) {
    updateDialogSlots();
  }
}

void DialogManager::draw(sf::RenderTarget &target, sf::RenderStates) const {
  if (_state != DialogManagerState::WaitingForChoice)
    return;

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = 534.f;

  auto actorName = _pPlayer->getActor();
  auto dialogHighlight = _pEngine->getVerbUiColors(actorName)->dialogHighlight;
  auto dialogNormal = _pEngine->getVerbUiColors(actorName)->dialogNormal;

  Text text;
  text.setFont(font);
  for (const auto &slot : _slots) {
    if (!slot.pChoice)
      continue;

    sf::String s;
    s = L"\u25CF ";
    s += slot.text;
    text.setString(s);
    text.setPosition(slot.pos.x, y + slot.pos.y);
    auto bounds = text.getGlobalBounds();
    text.setFillColor(bounds.contains(_mousePos) ? dialogHighlight : dialogNormal);
    target.draw(text);

    y += text.getGlobalBounds().height;
  }

  target.setView(view);
}

void DialogManager::update(const sf::Time &elapsed) {
  _pPlayer->update();
  auto oldState = _state;
  _state = _pPlayer->getState();

  if (oldState != _state && _state == DialogManagerState::WaitingForChoice) {
    updateDialogSlots();
  }

  updateChoices(elapsed);
}

void DialogManager::updateDialogSlots() {
  int i = 0;
  for (const auto &pStatement : _pPlayer->getChoices()) {
    if (pStatement) {
      auto pChoice = dynamic_cast<Ast::Choice *>(pStatement->expression.get());
      std::wstring dialogText = _pEngine->getText(pChoice->text);
      std::wregex re(L"(\\{([^\\}]*)\\})");
      std::wsmatch matches;
      if (std::regex_search(dialogText, matches, re)) {
        dialogText = matches.suffix();
      }
      _slots[i].text = dialogText;
      _slots[i].pos = {0, 0};
    }
    _slots[i].pChoice = pStatement;
    i++;
  }
}

void DialogManager::updateChoices(const sf::Time &elapsed) {
  if(_state != DialogManagerState::WaitingForChoice) return;

  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = 534.f;
  int dialog = 0;
  for (const auto &dlg : _slots) {
    if (dlg.pChoice == nullptr)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    sf::String s;
    s = L"\u25CF ";
    s += dlg.text;
    Text text;
    text.setFont(font);
    text.setPosition(dlg.pos.x, dlg.pos.y + y);
    text.setString(s);
    auto bounds = text.getGlobalBounds();
    if (bounds.width > Screen::Width) {
      if (bounds.contains(_mousePos)) {
        if ((bounds.width + dlg.pos.x) > Screen::Width) {
          dlg.pos.x -= SlidingSpeed * elapsed.asSeconds();
          if ((bounds.width + dlg.pos.x) < Screen::Width) {
            dlg.pos.x = Screen::Width - bounds.width;
          }
        }
      } else {
        if (dlg.pos.x < 0) {
          dlg.pos.x += SlidingSpeed * elapsed.asSeconds();
          if (dlg.pos.x > 0) {
            dlg.pos.x = 0;
          }
        }
      }
    }
    y += bounds.height;
    dialog++;
  }

  if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
    return;

  y = 534.f;
  dialog = 0;

  for (const auto &slot : _slots) {
    if (!slot.pChoice)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    sf::String s;
    s = L"\u25CF ";
    s += slot.text;
    Text text;
    text.setFont(font);
    text.setPosition(slot.pos.x, slot.pos.y + y);
    text.setString(s);
    if (text.getGlobalBounds().contains(_mousePos)) {
      choose(dialog + 1);
      break;
    }
    y += text.getGlobalBounds().height;
    dialog++;
  }
}

void DialogManager::choose(int choice) {
  if ((choice < 1) || (choice > static_cast<int>(_slots.size())))
    return;

  ScriptEngine::rawCall("onChoiceClick");
  _pPlayer->choose(choice);
}

void DialogManager::setMousePosition(sf::Vector2f pos) {
  _mousePos = pos;
}

} // namespace ng
