#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "YackTokenReader.h"

namespace gg
{
namespace Ast
{
class AstVisitor;

class Node
{
protected:
  Node() {}

public:
  virtual ~Node() {}
  virtual void accept(AstVisitor &visitor) = 0;

  std::streampos start;
  std::streampos end;
};
class Expression : public Node
{
protected:
  Expression() {}

public:
  virtual ~Expression() {}
};

class Condition : public Node
{
public:
  Condition() {}
  virtual ~Condition() {}

  virtual void accept(AstVisitor &visitor) override;

  std::string code;
};
class Statement : public Node
{
public:
  Statement() {}
  virtual ~Statement() {}

  virtual void accept(AstVisitor &visitor) override;

  std::unique_ptr<Expression> expression;
  std::vector<std::unique_ptr<Condition>> conditions;
};
class Goto : public Expression
{
public:
  Goto() {}
  virtual ~Goto() {}

  virtual void accept(AstVisitor &visitor) override;
  std::string name;
};
class Code : public Expression
{
public:
  Code() {}
  virtual ~Code() {}

  virtual void accept(AstVisitor &visitor) override;
  std::string code;
};
class Choice : public Expression
{
public:
  Choice() {}
  virtual ~Choice() {}

  virtual void accept(AstVisitor &visitor) override;
  int number;
  std::string text;
  std::unique_ptr<Goto> gotoExp;
};
class Say : public Expression
{
public:
  Say() {}
  virtual ~Say() {}

  virtual void accept(AstVisitor &visitor) override;
  std::string actor;
  std::string text;
};
class Pause : public Expression
{
public:
  Pause() {}
  virtual ~Pause() {}

  virtual void accept(AstVisitor &visitor) override;
  float time;
};
class Shutup : public Expression
{
public:
  Shutup() {}
  virtual ~Shutup() {}

  virtual void accept(AstVisitor &visitor) override;
};
class WaitFor : public Expression
{
public:
  WaitFor() {}
  virtual ~WaitFor() {}

  virtual void accept(AstVisitor &visitor) override;
  std::string actor;
};
class Label : public Node
{
public:
  Label() {}
  virtual ~Label() {}

  virtual void accept(AstVisitor &visitor) override;
  std::string name;
  std::vector<std::unique_ptr<Statement>> statements;
};
class CompilationUnit
{
public:
  CompilationUnit() {}
  virtual ~CompilationUnit() {}

  std::vector<std::unique_ptr<Label>> labels;
};
class AstVisitor
{
public:
  virtual void visit(const Statement &node);
  virtual void visit(const Label &node);
  virtual void visit(const Say &node);
  virtual void visit(const Choice &node);
  virtual void visit(const Code &node);
  virtual void visit(const Goto &node);
  virtual void visit(const Condition &node);
  virtual void visit(const Shutup &node);
  virtual void visit(const Pause &node);
  virtual void visit(const WaitFor &node);
  virtual void defaultVisit(const Node &node);
};
} // namespace Ast

class YackParser
{
public:
  YackParser(YackTokenReader &reader);
  std::unique_ptr<Ast::CompilationUnit> parse();

private:
  bool match(const std::initializer_list<TokenId> &ids);
  std::unique_ptr<Ast::Label> parseLabel();
  std::unique_ptr<Ast::Statement> parseStatement();
  std::unique_ptr<Ast::Condition> parseCondition();
  std::unique_ptr<Ast::Expression> parseExpression();
  std::unique_ptr<Ast::Say> parseSayExpression();
  std::unique_ptr<Ast::Expression> parseInstructionExpression();
  std::unique_ptr<Ast::Goto> parseGotoExpression();
  std::unique_ptr<Ast::Code> parseCodeExpression();
  std::unique_ptr<Ast::Choice> parseChoiceExpression();

private:
  YackTokenReader &_reader;
  YackTokenReader::iterator _it;
};
} // namespace gg
