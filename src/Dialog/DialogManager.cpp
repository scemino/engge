#include <regex>
#include "engge/Dialog/DialogManager.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include <ngf/Graphics/Text.h>
#include <engge/Util/Util.hpp>
#include <ngf/System/Mouse.h>
#include "engge/Graphics/Screen.hpp"

namespace ng {

static constexpr float SlidingSpeed = 25.f;

void DialogManager::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
  _pEngineDialogScript = std::make_unique<EngineDialogScript>(*pEngine);
  _pPlayer = std::make_unique<DialogPlayer>(*_pEngineDialogScript);
}

void DialogManager::start(const std::string &actor, const std::string &name, const std::string &node) {
  _pPlayer->start(actor, name, node);

  auto oldState = _state;
  _state = _pPlayer->getState();

  if (oldState != _state) {
    if (_state == DialogManagerState::WaitingForChoice) {
      updateDialogSlots();
    } else if (_state == DialogManagerState::None) {
      onDialogEnded();
    }
  }
}

void DialogManager::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (_state != DialogManagerState::WaitingForChoice)
    return;

  const auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = 534.f;

  auto actorName = _pPlayer->getActor();
  auto dialogHighlight = _pEngine->getVerbUiColors(actorName)->dialogHighlight;
  auto dialogNormal = _pEngine->getVerbUiColors(actorName)->dialogNormal;

  ngf::Text text;
  text.setFont(font);
  for (const auto &slot : _slots) {
    if (!slot.pChoice)
      continue;

    std::wstring s;
    s = L"\u25CF ";
    s += slot.text;
    text.setWideString(s);
    text.getTransform().setPosition({slot.pos.x, y + slot.pos.y});
    auto bounds = ng::getGlobalBounds(text);
    text.setColor(bounds.contains(_mousePos) ? dialogHighlight : dialogNormal);
    text.draw(target, {});

    y += ng::getGlobalBounds(text).getHeight();
  }

  target.setView(view);
}

void DialogManager::update(const ngf::TimeSpan &elapsed) {
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
      auto text = pChoice->text;
      if (!text.empty() && text[0] == '$') {
        text = _pEngine->executeDollar(text.substr(1));
      }
      std::wstring dialogText = ng::Engine::getText(text);
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

void DialogManager::updateChoices(const ngf::TimeSpan &elapsed) {
  if (_state != DialogManagerState::WaitingForChoice)
    return;

  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = 534.f;
  int dialog = 0;
  for (const auto &dlg : _slots) {
    if (dlg.pChoice == nullptr)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    std::wstring s;
    s = L"\u25CF ";
    s += dlg.text;
    ngf::Text text;
    text.setFont(font);
    text.getTransform().setPosition({dlg.pos.x, dlg.pos.y + y});
    text.setWideString(s);
    auto bounds = ng::getGlobalBounds(text);
    if (bounds.getWidth() > Screen::Width) {
      if (bounds.contains(_mousePos)) {
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
    y += bounds.getHeight();
    dialog++;
  }

  if (!ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left))
    return;

  y = 534.f;
  dialog = 0;

  for (const auto &slot : _slots) {
    if (!slot.pChoice)
      continue;

    // HACK: bad, bad, this code is the same as in the draw function
    std::wstring s;
    s = L"\u25CF ";
    s += slot.text;
    ngf::Text text;
    text.setFont(font);
    text.getTransform().setPosition({slot.pos.x, slot.pos.y + y});
    text.setWideString(s);
    if (ng::getGlobalBounds(text).contains(_mousePos)) {
      choose(dialog + 1);
      break;
    }
    y += ng::getGlobalBounds(text).getHeight();
    dialog++;
  }
}

void DialogManager::choose(int choice) {
  if ((choice < 1) || (choice > static_cast<int>(_slots.size())))
    return;

  ScriptEngine::rawCall("onChoiceClick");
  _pPlayer->choose(choice);
}

void DialogManager::setMousePosition(glm::vec2 pos) {
  _mousePos = pos;
}

void DialogManager::onDialogEnded() {
  ScriptEngine::call("onDialogEnded");
}

} // namespace ng
