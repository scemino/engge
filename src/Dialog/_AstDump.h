#pragma once
#include "Dialog/YackParser.h"

namespace gg
{
class _AstDump : public Ast::AstVisitor
{
public:
    static void dump(std::string filename)
    {
        YackTokenReader reader;
        reader.load(filename);
        std::cout << "# dump tokens: " << std::endl;
        for (auto it = reader.begin(); it != reader.end(); it++)
        {
            auto token = *it;
            std::cout << token << std::endl;
        }
        std::cout << "# dump AST: " << std::endl;
        YackParser parser(reader);
        auto pCu = parser.parse();
        _AstDump dump;
        for (const auto &label : pCu->labels)
        {
            label->accept(dump);
        }
    }

  private:
    virtual void visit(const Ast::Statement &node)
    {
        node.expression->accept(*this);
        for (const auto &cond : node.conditions)
        {
            cond->accept(*this);
        }
    }
    virtual void visit(const Ast::Pause &node)
    {
        std::cout << "pause: " << node.time << std::endl;
    }
    virtual void visit(const Ast::WaitFor &node)
    {
        std::cout << "waitfor " << node.actor << std::endl;
    }
    virtual void visit(const Ast::Shutup &node)
    {
        std::cout << "shutup " << std::endl;
    }
    virtual void visit(const Ast::Label &node)
    {
        std::cout << "label " << node.name << ":" << std::endl;
        for (const auto &statement : node.statements)
        {
            statement->accept(*this);
        }
    }
    virtual void visit(const Ast::Say &node)
    {
        std::cout << "say " << node.actor << ": " << node.text << std::endl;
    }
    virtual void visit(const Ast::Choice &node)
    {
        std::cout << "choice " << node.number << " " << node.text << std::endl;
    }
    virtual void visit(const Ast::Code &node)
    {
        std::cout << "code " << node.code << std::endl;
    }
    virtual void visit(const Ast::Goto &node)
    {
        std::cout << "goto " << node.name << std::endl;
    }
    virtual void visit(const Ast::Condition &node)
    {
        std::cout << "condition: " << node.code << std::endl;
    }
};
} // namespace gg
