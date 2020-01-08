#include <regex>
#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Logger.h"
#include "Preferences.h"
#include "ScriptEngine.h"
#include "Text.h"
#include "_SayFunction.h"

namespace ng
{
DialogManager::DialogManager()
    : _dialogVisitor(*this)
{
    for (auto &dlg : _dialog)
    {
        dlg.id = 0;
    }
}

void DialogManager::setEngine(Engine *pEngine)
{
    _pEngine = pEngine;
    _dialogVisitor.setEngine(_pEngine);
    _font.setTextureManager(&pEngine->getTextureManager());
    _font.setSettings(&pEngine->getSettings());
    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
    _font.load(retroFonts ? "FontRetroSheet": "FontModernSheet");
}

void DialogManager::addFunction(std::unique_ptr<Function> function)
{
    _functions.push_back(std::move(function));
}

void DialogManager::start(const std::string &name, const std::string &node)
{
    _actorName.clear();
    std::string path;
    path.append(name).append(".byack");

    trace("start dialog {} from node {}", name, node);

    YackTokenReader reader;
    reader.setSettings(_pEngine->getSettings());
    reader.load(path);
    YackParser parser(reader);
    _pCompilationUnit = parser.parse();

    selectLabel(node);
}

void DialogManager::selectLabel(const std::string &name)
{
    trace("select label {}", name);
    _state = DialogManagerState::None;
    for (auto &line : _dialog)
    {
        line.id = 0;
    }
    auto it = std::find_if(_pCompilationUnit->labels.begin(), _pCompilationUnit->labels.end(), [&name](const std::unique_ptr<Ast::Label> &label) {
        return label->name == name;
    });
    _pLabel = it != _pCompilationUnit->labels.end() ? it->get() : nullptr;
    if (_pLabel)
    {
        _pLabel->accept(_dialogVisitor);
    }
    if(!_functions.empty())
    {
        _state = DialogManagerState::Active;
    }
    else if(std::any_of(_dialog.begin(),_dialog.end(),[](auto& line){ return line.id != 0; }))
    {
        _state = DialogManagerState::WaitingForChoice;
    }
    
    if (_pLabel && _state == DialogManagerState::None)
    {
        it++;
        if (it != _pCompilationUnit->labels.end())
            selectLabel(it->get()->name);
    }
}

void DialogManager::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_functions.empty())
        return;

    int dialog = 0;
    auto screen = target.getView().getSize();
    auto scale = (screen.y * 5.f) / (8.f * 512.f);

    NGText text;
    text.scale(scale, scale);
    text.setFont(_font);
    for (auto &dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        text.setPosition(0, screen.y - 3 * screen.y / 14.f + dialog * 6);
        std::wstring dialogText = dlg.text;
        std::wregex re(L"(\\{([^\\}]*)\\})");
        std::wsmatch matches;
        if (std::regex_search(dialogText, matches, re))
        {
            dialogText = matches.suffix();
        }
        
        sf::String s;
        s = L"● ";
        s += dialogText;
        text.setText(s);
        text.setColor(text.getBoundRect().contains(_pEngine->getMousePos()) ? _pEngine->getVerbUiColors(_actorName)->dialogHighlight : _pEngine->getVerbUiColors(_actorName)->dialogNormal);
        target.draw(text, states);
        dialog++;
    }
}

void DialogManager::update(const sf::Time &elapsed)
{
    auto screen = _pEngine->getWindow().getView().getSize();
    auto scale = (screen.y * 5.f) / (8.f * 512.f);

    if(!_functions.empty())
    {
        _state = DialogManagerState::Active;
    }
    else
    {
        _state = std::any_of(_dialog.begin(),_dialog.end(),[](auto& line){ return line.id != 0; }) ? 
            DialogManagerState::WaitingForChoice : DialogManagerState::None;
    }

    if (!_functions.empty())
    {
        if (_functions[0]->isElapsed())
            _functions.erase(_functions.begin());
        else
            (*_functions[0])(elapsed);
        return;
    }

    if (_pLabel && _state == DialogManagerState::None)
    {
        auto name = _pLabel->name;
        auto it = std::find_if(_pCompilationUnit->labels.begin(), _pCompilationUnit->labels.end(), [&name](const std::unique_ptr<Ast::Label> &label) {
            return label->name == name;
        });
        it++;
        if (it != _pCompilationUnit->labels.end())
            selectLabel(it->get()->name);
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        return;

    int dialog = 0;
    for (const auto& dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        // HACK: bad, bad, this code is the same as in the draw function
        NGText text;
        text.scale(scale, scale);
        text.setFont(_font);
        text.setPosition(0, screen.y - 3 * screen.y / 14.f + dialog * 6);
        sf::String s;
        s = L"● ";
        s += dlg.text;
        text.setText(s);
        if (text.getBoundRect().contains(_pEngine->getMousePos()))
        {
            choose(dialog + 1);
            break;
        }
        dialog++;
    }
}

void DialogManager::setActorName(const std::string& actor)
{
    _actorName = actor;
}

void DialogManager::choose(int choice)
{
    if((choice < 1) || (choice > _dialog.size())) return;

    size_t i = 0;
    for(const auto& dlg : _dialog)
    {
        if(dlg.id == 0) continue;
        if((choice-1) == i)
        {
            ScriptEngine::call("onChoiceClick");
            std::ostringstream os;
            os << '@' << dlg.id;
            if(_parrotModeEnabled) {
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
