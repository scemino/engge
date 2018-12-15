#pragma once

#include "YackParser.h"

namespace ng
{
class NGEngine;
class DialogManager
{
  public:
    DialogManager(NGEngine &engine);
    void start(const std::string &name);
    void selectLabel(const std::string &label);

  private:
    NGEngine &_engine;
    std::unique_ptr<Ast::CompilationUnit> _pCompilationUnit;
    Ast::Label *_pLabel;
};
} // namespace ng
