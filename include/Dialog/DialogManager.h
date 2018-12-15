#pragma once

#include "YackParser.h"

namespace gg
{
class GGEngine;
class DialogManager
{
  public:
    DialogManager(GGEngine &engine);
    void start(const std::string &name);
    void selectLabel(const std::string &label);

  private:
    GGEngine &_engine;
    std::unique_ptr<Ast::CompilationUnit> _pCompilationUnit;
    Ast::Label *_pLabel;
};
} // namespace gg
