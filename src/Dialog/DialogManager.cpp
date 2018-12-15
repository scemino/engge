
#include "Dialog/DialogManager.h"
#include "GGEngine.h"
#include "_DialogVisitor.h"

namespace gg
{
DialogManager::DialogManager(GGEngine &engine)
    : _engine(engine)
{
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
    auto &dlg = _engine.getDialog();
    for (auto &line : dlg)
    {
        line.id = 0;
    }
    auto it = std::find_if(_pCompilationUnit->labels.begin(), _pCompilationUnit->labels.end(), [&name](const std::unique_ptr<Ast::Label> &label) {
        return label->name == name;
    });
    _pLabel = it != _pCompilationUnit->labels.end() ? it->get() : nullptr;
    if (_pLabel)
    {
        _DialogVisitor visitor(_engine, *this);
        _pLabel->accept(visitor);
    }
}
} // namespace gg
