
#include "Dialog/DialogManager.h"
#include "NGEngine.h"
#include "NGActor.h"
#include "_SayFunction.h"

namespace ng
{
DialogManager::DialogManager(NGEngine &engine)
    : _engine(engine), _isActive(false), _dialogVisitor(_engine, *this)
{
    for (auto &dlg : _dialog)
    {
        dlg.id = 0;
    }
}

void DialogManager::start(const std::string &name)
{
    std::string path(_engine.getSettings().getGamePath());
    path.append(name).append(".byack");

    YackTokenReader reader;
    reader.load(path);
    YackParser parser(reader);
    _pCompilationUnit = std::move(parser.parse());

    selectLabel("main");
}

void DialogManager::selectLabel(const std::string &name)
{
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
    for (auto &line : _dialog)
    {
        if (line.id != 0)
        {
            _isActive = true;
            break;
        }
    }
}

void DialogManager::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!_functions.empty())
        return;

    int dialog = 0;
    NGText text;
    text.setAlignment(NGTextAlignment::Left);
    text.setFont(_engine.getFont());
    for (auto dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        text.setPosition(0, Screen::Height - 3 * Screen::Height / 14.f + dialog * 10);
        text.setText(dlg.text);
        text.setColor(text.getBoundRect().contains(_engine.getMousePos()) ? _engine.getVerbUiColors(0).dialogHighlight : _engine.getVerbUiColors(0).dialogNormal);
        target.draw(text, states);
        dialog++;
    }
}

void DialogManager::update(const sf::Time &elapsed)
{
    if (!_functions.empty())
    {
        if (_functions[0]->isElapsed())
            _functions.erase(_functions.begin());
        else
            (*_functions[0])();
        return;
    }

    sf::Mouse m;
    if (!m.isButtonPressed(sf::Mouse::Button::Left))
        return;

    int dialog = 0;
    for (auto dlg : _dialog)
    {
        if (dlg.id == 0)
            continue;

        NGText text;
        text.setFont(_engine.getFont());
        text.setPosition(0, Screen::Height - 3 * Screen::Height / 14.f + dialog * 10);
        text.setText(dlg.text);
        if (text.getBoundRect().contains(_engine.getMousePos()))
        {
            auto say = std::make_unique<_SayFunction>(*_engine.getCurrentActor(), dlg.id);
            _functions.push_back(std::move(say));
            _dialogVisitor.select(*dlg.pChoice);
            selectLabel(dlg.label);
            break;
        }
        dialog++;
    }
}

} // namespace ng
