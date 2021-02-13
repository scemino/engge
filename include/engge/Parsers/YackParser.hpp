#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "YackTokenReader.hpp"

namespace ng {
namespace Ast {
class AstVisitor;

class Node {
protected:
  Node() = default;

public:
  virtual ~Node();
  virtual void accept(AstVisitor &visitor) = 0;

  std::streampos start;
  std::streampos end;
};

class Expression : public Node {
protected:
  Expression() = default;

public:
  ~Expression() override;
};

class Condition : public Node {
protected:
  explicit Condition(int32_t line);
public:
  ~Condition() override;
  int32_t getLine() const { return m_line; }
private:
  int32_t m_line;
};

class CodeCondition : public Condition {
public:
  explicit CodeCondition(int32_t line);
  ~CodeCondition() override;

  void accept(AstVisitor &visitor) override;

  std::string code;
};

class OnceCondition : public Condition {
public:
  explicit OnceCondition(int32_t line);
  ~OnceCondition() override;

  void accept(AstVisitor &visitor) override;
};

class ShowOnceCondition : public Condition {
public:
  explicit ShowOnceCondition(int32_t line);
  ~ShowOnceCondition() override;

  void accept(AstVisitor &visitor) override;
};

class OnceEverCondition : public Condition {
public:
  explicit OnceEverCondition(int32_t line);
  ~OnceEverCondition() override;

  void accept(AstVisitor &visitor) override;
};

class TempOnceCondition : public Condition {
public:
  explicit TempOnceCondition(int32_t line);
  ~TempOnceCondition() override;

  void accept(AstVisitor &visitor) override;
};

class Statement : public Node {
public:
  Statement() = default;
  ~Statement() override;

  void accept(AstVisitor &visitor) override;

  std::unique_ptr<Expression> expression;
  std::vector<std::unique_ptr<Condition>> conditions;
};

class Goto : public Expression {
public:
  Goto() = default;
  ~Goto() override;

  void accept(AstVisitor &visitor) override;
  std::string name;
};

class Code : public Expression {
public:
  Code() = default;
  ~Code() override;

  void accept(AstVisitor &visitor) override;
  std::string code;
};

class Choice : public Expression {
public:
  Choice() = default;
  ~Choice() override;

  void accept(AstVisitor &visitor) override;
  int number{0};
  std::string text;
  std::unique_ptr<Goto> gotoExp;
};

class Say : public Expression {
public:
  Say() = default;
  ~Say() override;

  void accept(AstVisitor &visitor) override;
  std::string actor;
  std::string text;
};

class Pause : public Expression {
public:
  Pause() = default;
  ~Pause() override;

  void accept(AstVisitor &visitor) override;
  float time{0};
};

class Parrot : public Expression {
public:
  Parrot() = default;
  ~Parrot() override;

  void accept(AstVisitor &visitor) override;
  bool active{true};
};

class Dialog : public Expression {
public:
  Dialog() = default;
  ~Dialog() override;

  void accept(AstVisitor &visitor) override;
  std::string actor;
};

class Override : public Expression {
public:
  Override() = default;
  ~Override() override;

  void accept(AstVisitor &visitor) override;
  std::string node;
};

class Shutup : public Expression {
public:
  Shutup() = default;
  ~Shutup() override;

  void accept(AstVisitor &visitor) override;
};

class AllowObjects : public Expression {
public:
  AllowObjects() = default;
  ~AllowObjects() override;

  void accept(AstVisitor &visitor) override;

  bool allow{true};
};

class Limit : public Expression {
public:
  Limit() = default;
  ~Limit() override;

  void accept(AstVisitor &visitor) override;

  int max{0};
};

class WaitWhile : public Expression {
public:
  WaitWhile() = default;
  ~WaitWhile() override;

  void accept(AstVisitor &visitor) override;

  std::string condition;
};

class WaitFor : public Expression {
public:
  WaitFor() = default;
  ~WaitFor() override;

  void accept(AstVisitor &visitor) override;
  std::string actor;
};

class Label : public Node {
public:
  Label() = default;
  ~Label() override;

  void accept(AstVisitor &visitor) override;
  std::string name;
  std::vector<std::unique_ptr<Statement>> statements;
};

class CompilationUnit {
public:
  CompilationUnit() = default;
  virtual ~CompilationUnit();

  std::vector<std::unique_ptr<Label>> labels;
};

class AstVisitor {
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
  virtual void visit(const TempOnceCondition &node);
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

class YackParser {
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
  YackTokenReader &m_reader;
  YackTokenReader::iterator m_it;
};
} // namespace ng
