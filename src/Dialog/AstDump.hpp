#pragma once
#include "engge/Parsers/YackParser.hpp"

namespace ng {
class AstDump : public Ast::AstVisitor {
public:
  static void dump(const std::string &filename) {
    YackTokenReader reader;
    reader.load(filename);
    std::cout << "# dump tokens: " << std::endl;
    for (auto it = reader.begin(); it != reader.end(); it++) {
      auto token = *it;
      auto text = reader.readText(token);
      std::cout << token << " " << text << std::endl;
    }
    std::cout << "# dump AST: " << std::endl;
    YackParser parser(reader);
    auto pCu = parser.parse();
    _AstDump dump;
    for (const auto &label : pCu->labels) {
      label->accept(dump);
    }
  }

private:
  void visit(const Ast::Statement &node) override {
    node.expression->accept(*this);
    for (const auto &cond : node.conditions) {
      cond->accept(*this);
    }
  }
  void visit(const Ast::Pause &node) override {
    std::cout << "pause: " << node.time << std::endl;
  }
  void visit(const Ast::WaitFor &node) override {
    std::cout << "waitfor " << node.actor << std::endl;
  }
  void visit(const Ast::Parrot &node) override {
    std::cout << "parrot " << node.active << std::endl;
  }
  void visit(const Ast::Dialog &node) override {
    std::cout << "dialog " << node.actor << std::endl;
  }
  void visit(const Ast::Shutup &) override {
    std::cout << "shutup " << std::endl;
  }
  void visit(const Ast::Override &node) override {
    std::cout << "override " << node.node << std::endl;
  }
  void visit(const Ast::Label &node) override {
    std::cout << "label " << node.name << ":" << std::endl;
    for (const auto &statement : node.statements) {
      statement->accept(*this);
    }
  }
  void visit(const Ast::Say &node) override {
    std::cout << "say " << node.actor << ": " << node.text << std::endl;
  }
  void visit(const Ast::Choice &node) override {
    std::cout << "choice " << node.number << " " << node.text << std::endl;
  }
  void visit(const Ast::Code &node) override {
    std::cout << "code " << node.code << std::endl;
  }
  void visit(const Ast::Goto &node) override {
    std::cout << "goto " << node.name << std::endl;
  }
  void visit(const Ast::OnceCondition & node) override {
    std::cout << "condition: once (" << node.getLine() << ")" << std::endl;
  }
  void visit(const Ast::ShowOnceCondition &node) override {
    std::cout << "condition: showonce (" << node.getLine() << ")" << std::endl;
  }
  void visit(const Ast::CodeCondition &node) override {
    std::cout << "condition: " << node.code << "(" << node.getLine() << ")" << std::endl;
  }
  void visit(const Ast::TempOnceCondition &node) override {
    std::cout << "condition: temponce (" << node.getLine() << ")" << std::endl;
  }
};
} // namespace ng
