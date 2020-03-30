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
    _state = DialogManagerState::None;
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
        _pLabel->accept(_dialogVisitor);
    }
    if (!_functions.empty()) {
        _state = DialogManagerState::Active;
    } else if (std::any_of(_dialog.begin(), _dialog.end(), [](auto &line) { return line.id != 0; })) {
        _state = DialogManagerState::WaitingForChoice;
    }

    if (_pLabel && _state == DialogManagerState::None) {
        it++;
        if (it != _pCompilationUnit->labels.end())
            selectLabel(it->get()->name);
    }
}

void DialogManager::draw(sf::RenderTarget &target, sf::RenderStates) const {
    if (getState() != DialogManagerState::WaitingForChoice)
        return;

    if (!_functions.empty())
        return;

    const auto &win = _pEngine->getWindow();
    auto pos = win.mapPixelToCoords(sf::Mouse::getPosition(win),
                                    sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    const auto view = target.getView();
    target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    int dialog = 0;

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                   PreferenceDefaultValues::RetroFonts);
    const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

    auto y = 534.f;

    auto dialogHighlight = _pEngine->getVerbUiColors(_actorName)->dialogHighlight;
    auto dialogNormal = _pEngine->getVerbUiColors(_actorName)->dialogNormal;

    Text text;
    text.setFont(font);
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
        text.setPosition(0, y);
        auto bounds = text.getGlobalBounds();
        text.setFillColor(bounds.contains(pos) ? dialogHighlight : dialogNormal);
        target.draw(text);

        y += text.getGlobalBounds().height;
        dialog++;
    }

    target.setView(view);
}

void DialogManager::update(const sf::Time &elapsed) {
    const auto &win = _pEngine->getWindow();
    auto pos = win.mapPixelToCoords(sf::Mouse::getPosition(win),
                                    sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    if (!_functions.empty()) {
        _state = DialogManagerState::Active;
    } else {
        _state = std::any_of(_dialog.begin(), _dialog.end(), [](auto &line) { return line.id != 0; }) ?
                 DialogManagerState::WaitingForChoice : DialogManagerState::None;
    }

    if (!_functions.empty()) {
        if (_functions[0]->isElapsed())
            _functions.erase(_functions.begin());
        else
            (*_functions[0])(elapsed);
        return;
    }

    if (_pLabel && _state == DialogManagerState::None) {
        auto name = _pLabel->name;
        auto it = std::find_if(_pCompilationUnit->labels.begin(),
                               _pCompilationUnit->labels.end(),
                               [&name](const std::unique_ptr<Ast::Label> &label) {
                                 return label->name == name;
                               });
        it++;
        if (it != _pCompilationUnit->labels.end())
            selectLabel(it->get()->name);
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        return;

    int dialog = 0;
    auto y = 534.f;
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
        text.setPosition(0, y);
        text.setString(s);
        if (text.getGlobalBounds().contains(pos)) {
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
                auto say = std::make_unique<_SayFunction>(*_pEngine->getCurrentActor(), os.str());
                _functions.push_back(std::move(say));
            }
            _dialogVisitor.select(*dlg.pChoice);
            selectLabel(dlg.label);
            return;
        }
        i++;
    }
}

} // namespace ng
