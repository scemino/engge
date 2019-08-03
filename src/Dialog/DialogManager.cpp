
#include "Actor.h"
#include "Dialog/DialogManager.h"
#include "Engine.h"
#include "Logger.h"
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
    _font.load("FontModernSheet");
}

void DialogManager::addFunction(std::unique_ptr<Function> function)
{
    _functions.push_back(std::move(function));
}

void DialogManager::start(const std::string &name, const std::string &node)
{
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
    _isActive = false;
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
    _isActive = !_functions.empty();
    _isActive |= std::any_of(_dialog.begin(),_dialog.end(),[](auto& line){ return line.id != 0; });
    
    if (_pLabel && !_isActive)
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
    NGText text;
    text.setFont(_font);
    for (auto &dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        text.setPosition(0, screen.y - 3 * screen.y / 14.f + dialog * 6);
        sf::String s;
        s = L"● ";
        s += dlg.text;
        text.setText(s);
        text.setColor(text.getBoundRect().contains(_pEngine->getMousePos()) ? _pEngine->getVerbUiColors(0).dialogHighlight : _pEngine->getVerbUiColors(0).dialogNormal);
        target.draw(text, states);
        dialog++;
    }
}

void DialogManager::update(const sf::Time &elapsed)
{
    auto screen = _pEngine->getWindow().getView().getSize();
    _isActive = !_functions.empty();
    _isActive |= std::any_of(_dialog.begin(),_dialog.end(),[](auto& line){ return line.id != 0; });

    if (!_functions.empty())
    {
        if (_functions[0]->isElapsed())
            _functions.erase(_functions.begin());
        else
            (*_functions[0])(elapsed);
        return;
    }

    if (_pLabel && !_isActive)
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
    for (auto dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        // HACK: bad, bad, this code is the same as in the draw function
        NGText text;
        text.setFont(_font);
        text.setPosition(0, screen.y - 3 * screen.y / 14.f + dialog * 6);
        sf::String s;
        s = L"● ";
        s += dlg.text;
        text.setText(s);
        if (text.getBoundRect().contains(_pEngine->getMousePos()))
        {
            auto say = std::make_unique<_SayFunction>(*_pEngine->getCurrentActor(), dlg.id);
            _functions.push_back(std::move(say));
            _dialogVisitor.select(*dlg.pChoice);
            selectLabel(dlg.label);
            break;
        }
        dialog++;
    }
}

} // namespace ng
