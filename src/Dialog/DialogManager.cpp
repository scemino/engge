#include <regex>
#include "Dialog/DialogManager.hpp"
#include "Engine/Engine.hpp"
#include "System/Logger.hpp"
#include "Engine/Preferences.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Graphics/Text.hpp"
#include "_SayFunction.hpp"
#include "Graphics/Screen.hpp"

namespace ng {
DialogManager::DialogManager()
    : _dialogVisitor(*this) {
  for (auto &dlg : _dialog) {
    dlg.id = 0;
  }
}

void DialogManager::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
  _dialogVisitor.setEngine(_pEngine);
}

void DialogManager::addFunction(std::unique_ptr<Function> function) {
  _functions.push_back(std::move(function));
}

void DialogManager::start(const std::string &name, const std::string &node) {
  _actorName.clear();
  _parrotModeEnabled = true;
  _limit = 6;
  _override.clear();

  std::string path;
  path.append(name).append(".byack");

  trace("start dialog {} from node {}", name, node);

  YackTokenReader reader;
  reader.load(path);
  YackParser parser(reader);
  _pCompilationUnit = parser.parse();

  selectLabel(node);
}

void DialogManager::selectLabel(const std::string &name) {
  trace("select label {}", name);
  _state = DialogManagerState::Active;
  for (auto &line : _dialog) {
    line.id = 0;
  }
  auto it = std::find_if(_pCompilationUnit->labels.begin(),
                         _pCompilationUnit->labels.end(),
                         [&name](const std::unique_ptr<Ast::Label> &label) {
                           return label->name == name;
                         });
  _pLabel = it != _pCompilationUnit->labels.end() ? it->get() : nullptr;
  if (_pLabel) {
    _currentStatement = _pLabel->statements.begin();
  }
  update(sf::seconds(0));
}

Actor* DialogManager::getTalkingActor() {
  if(_actorName.empty()) return _pEngine->getCurrentActor();
  for (auto &actor : _pEngine->getActors()) {
    if (actor->getKey() == _actorName) {
      return actor.get();
    }
  }
  return nullptr;
}

void DialogManager::draw(sf::RenderTarget &target, sf::RenderStates) const {
  if (getState() != DialogManagerState::WaitingForChoice)
    return;

  if (!_functions.empty())
    return;

  const auto view = target.getView();
  target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));


  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  auto y = 534.f;

  auto dialogHighlight = _pEngine->getVerbUiColors(_actorName)->dialogHighlight;
  auto dialogNormal = _pEngine->getVerbUiColors(_actorName)->dialogNormal;

  Text text;
  text.setFont(font);
  int dialog = 0;
  for (auto &dlg : _dialog) {
    if (dlg.id == 0)
      continue;

    if ((dialog + 1) >= _limit)
      break;

    std::wstring dialogText = dlg.text;
    std::wregex re(L"(\\{([^\\}]*)\\})");
    std::wsmatch matches;
    if (std::regex_search(dialogText, matches, re)) {
      dialogText = matches.suffix();
    }

    sf::String s;
    s = L"\u25CF ";
    s += dialogText;
    text.setString(s);
    text.setPosition(dlg.pos.x, y + dlg.pos.y);
    auto bounds = text.getGlobalBounds();
    text.setFillColor(bounds.contains(_mousePos) ? dialogHighlight : dialogNormal);
    target.draw(text);

    y += text.getGlobalBounds().height;
    dialog++;
  }

  target.setView(view);
}

void DialogManager::update(const sf::Time &elapsed) {
  auto hasChoice = std::any_of(_dialog.cbegin(), _dialog.cend(), [](const auto &line) { return line.id != 0; });
  if (hasChoice) {
    _state = DialogManagerState::WaitingForChoice;
  } else if (!_functions.empty()) {
    _state = DialogManagerState::Active;
  } else if (!_pLabel) {
    _state = DialogManagerState::None;
    return;
  } else if (_currentStatement == _pLabel->statements.end()) {
    // jump to next label
    auto name = _pLabel->name;
    auto it = std::find_if(_pCompilationUnit->labels.begin(),
                           _pCompilationUnit->labels.end(),
                           [&name](const std::unique_ptr<Ast::Label> &label) {
                             return label->name == name;
                           });
    it++;
    if (it != _pCompilationUnit->labels.end())
      selectLabel(it->get()->name);
    else
      _state = DialogManagerState::None;
    return;
  } else {
    bool isChoice;
    _state = DialogManagerState::Active;
    do {
      auto pStatement = _currentStatement->get();
      pStatement->accept(_dialogVisitor);
      isChoice = dynamic_cast<Ast::Choice *>(pStatement->expression.get()) != nullptr;
      _currentStatement++;
    } while (_functions.empty() && isChoice && _currentStatement != _pLabel->statements.end());
  }

  if (_state == DialogManagerState::Active) {
    if (_functions.empty())
      return;
    if (_functions[0]->isElapsed())
      _functions.erase(_functions.begin());
    else
      (*_functions[0])(elapsed);
    return;
  }

  auto y = 534.f;
  int dialog = 0;
  for (const auto &dlg : _dialog) {
    if (dlg.id == 0)
      continue;

    if ((dialog + 1) >= _limit)
      break;

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                   PreferenceDefaultValues::RetroFonts);
    const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

    // HACK: bad, bad, this code is the same as in the draw function
    sf::String s;
    s = L"\u25CF ";
    s += dlg.text;
    Text text;
    text.setFont(font);
    text.setPosition(dlg.pos.x, dlg.pos.y + y);
    text.setString(s);
    auto bounds = text.getGlobalBounds();
    if(bounds.width > Screen::Width) {
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
          if (dlg.pos.x> 0) {
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
  for (const auto &dlg : _dialog) {
    if (dlg.id == 0)
      continue;

    if ((dialog + 1) >= _limit)
      break;

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                   PreferenceDefaultValues::RetroFonts);
    const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

    // HACK: bad, bad, this code is the same as in the draw function
    sf::String s;
    s = L"\u25CF ";
    s += dlg.text;
    Text text;
    text.setFont(font);
    text.setPosition(dlg.pos.x, dlg.pos.y + y);
    text.setString(s);
    if (text.getGlobalBounds().contains(_mousePos)) {
      choose(dialog + 1);
      break;
    }
    y += text.getGlobalBounds().height;
    dialog++;
  }
}

void DialogManager::setActorName(const std::string &actor) {
  _actorName = actor;
}

void DialogManager::choose(int choice) {
  if ((choice < 1) || (choice > static_cast<int>(_dialog.size())))
    return;

  int i = 0;
  for (const auto &dlg : _dialog) {
    if (dlg.id == 0)
      continue;
    if ((choice - 1) == i) {
      ScriptEngine::rawCall("onChoiceClick");
      std::ostringstream os;
      os << '@' << dlg.id;
      if (_parrotModeEnabled) {
        auto say = std::make_unique<_SayFunction>(*getTalkingActor(), os.str());
        _functions.push_back(std::move(say));
      }
      _dialogVisitor.select(*dlg.pChoice);
      selectLabel(dlg.label);
      return;
    }
    i++;
  }
}

void DialogManager::setMousePosition(sf::Vector2f pos) {
  _mousePos = pos;
}

} // namespace ng
