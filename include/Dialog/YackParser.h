#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "YackTokenReader.h"

namespace ng
{
namespace Ast
{
class AstVisitor;

class Node
{
protected:
  Node() {}

public:
  virtual ~Node();
  virtual void accept(AstVisitor &visitor) = 0;

  std::streampos start;
  std::streampos end;
};
class Expression : public Node
{
protected:
  Expression() {}

public:
  virtual ~Expression() override;
};

class Condition : public Node
{
protected:
  Condition() {}
public:
  virtual ~Condition() override;
};
class CodeCondition : public Condition
{
public:
  CodeCondition() {}
  virtual ~CodeCondition() override;

  virtual void accept(AstVisitor &visitor) override;

  std::string code;
};
class OnceCondition : public Condition
{
public:
  OnceCondition() {}
  virtual ~OnceCondition() override;  

  virtual void accept(AstVisitor &visitor) override;
};
class ShowOnceCondition : public Condition
{
public:
  ShowOnceCondition() {}
  virtual ~ShowOnceCondition() override;

  virtual void accept(AstVisitor &visitor) override;
};
class OnceEverCondition : public Condition
{
public:
  OnceEverCondition() {}
  virtual ~OnceEverCondition() override;

  virtual void accept(AstVisitor &visitor) override;
};
class Statement : public Node
{
public:
  Statement() {}
  virtual ~Statement() override;

  virtual void accept(AstVisitor &visitor) override;

  std::unique_ptr<Expression> expression;
  std::vector<std::unique_ptr<Condition>> conditions;
};
class Goto : public Expression
{
public:
  Goto() {}
  virtual ~Goto() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string name;
};
class Code : public Expression
{
public:
  Code() {}
  virtual ~Code() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string code;
};
class Choice : public Expression
{
public:
  Choice() {}
  virtual ~Choice() override;

  virtual void accept(AstVisitor &visitor) override;
  int number{0};
  std::string text;
  std::unique_ptr<Goto> gotoExp;
};
class Say : public Expression
{
public:
  Say() {}
  virtual ~Say() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string actor;
  std::string text;
};
class Pause : public Expression
{
public:
  Pause() {}
  virtual ~Pause() override;

  virtual void accept(AstVisitor &visitor) override;
  float time{0};
};
class Parrot : public Expression
{
public:
  Parrot() {}
  virtual ~Parrot() override;

  virtual void accept(AstVisitor &visitor) override;
  bool active{true};
};
class Dialog : public Expression
{
public:
  Dialog() {}
  virtual ~Dialog() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string actor;
};
class Override : public Expression
{
public:
  Override() {}
  virtual ~Override() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string node;
};
class Shutup : public Expression
{
public:
  Shutup() {}
  virtual ~Shutup() override;

  virtual void accept(AstVisitor &visitor) override;
};
class AllowObjects : public Expression
{
public:
  AllowObjects() {}
  virtual ~AllowObjects() override;

  virtual void accept(AstVisitor &visitor) override;
  
  bool allow{true};
};
class Limit : public Expression
{
public:
  Limit() {}
  virtual ~Limit() override;

  virtual void accept(AstVisitor &visitor) override;
  
  int max{0};
};
class WaitWhile : public Expression
{
public:
  WaitWhile() {}
  virtual ~WaitWhile() override;

  virtual void accept(AstVisitor &visitor) override;

  std::string condition;
};
class WaitFor : public Expression
{
public:
  WaitFor() {}
  virtual ~WaitFor() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string actor;
};
class Label : public Node
{
public:
  Label() {}
  virtual ~Label() override;

  virtual void accept(AstVisitor &visitor) override;
  std::string name;
  std::vector<std::unique_ptr<Statement>> statements;
};
class CompilationUnit
{
public:
  CompilationUnit() {}
  virtual ~CompilationUnit();

  std::vector<std::unique_ptr<Label>> labels;
};
class AstVisitor
{
public:
  virtual ~AstVisitor();
  virtual void visit(const Statement &node);
  virtual void visit(const Label &node);
  virtual void visit(const Say &node);
  virtual void visit(const Choice &node);
  virtual void visit(const Code &node);
  virtual void visit(const Goto &node);
  virtual void visit(const CodeCondition &node);
  virtual void visit(const OnceCondition &node);
  virtual void visit(const ShowOnceCondition &node);
  virtual void visit(const OnceEverCondition &node);
  virtual void visit(const Shutup &node);
  virtual void visit(const Pause &node);
  virtual void visit(const WaitFor &node);
  virtual void visit(const Parrot &node);
  virtual void visit(const Dialog &node);
  virtual void visit(const Override &node);
  virtual void visit(const AllowObjects &node);
  virtual void visit(const WaitWhile &node);
  virtual void visit(const Limit &node);
  virtual void defaultVisit(const Node &node);
};
} // namespace Ast

class YackParser
{
public:
  explicit YackParser(YackTokenReader &reader);
  std::unique_ptr<Ast::CompilationUnit> parse();

private:
  bool match(const std::initializer_list<TokenId> &ids);
  std::unique_ptr<Ast::Label> parseLabel();
  std::unique_ptr<Ast::Statement> parseStatement();
  std::unique_ptr<Ast::Condition> parseCondition();
  std::unique_ptr<Ast::Expression> parseExpression();
  std::unique_ptr<Ast::Say> parseSayExpression();
  std::unique_ptr<Ast::Expression> parseWaitWhileExpression();
  std::unique_ptr<Ast::Expression> parseInstructionExpression();
  std::unique_ptr<Ast::Goto> parseGotoExpression();
  std::unique_ptr<Ast::Code> parseCodeExpression();
  std::unique_ptr<Ast::Choice> parseChoiceExpression();

private:
  YackTokenReader &_reader;
  YackTokenReader::iterator _it;
};
} // namespace ng
