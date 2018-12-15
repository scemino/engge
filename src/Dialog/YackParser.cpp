#include "Dialog/YackParser.h"

namespace gg
{
std::ostream &operator<<(std::ostream &os, const Token &token)
{
    return os << "[" << token.start << "," << token.end << "] " << token.readToken();
}

YackParser::YackParser(YackTokenReader &reader)
    : _reader(reader), _it(_reader.begin())
{
}

std::unique_ptr<Ast::CompilationUnit> YackParser::parse()
{
    auto pCu = std::make_unique<Ast::CompilationUnit>();
    while (!match({TokenId::End}))
    {
        pCu->labels.push_back(parseLabel());
    }
    return pCu;
}

std::unique_ptr<Ast::Label> YackParser::parseLabel()
{
    auto pLabel = std::make_unique<Ast::Label>();

    // skip empty lines
    while (match({TokenId::NewLine}))
        _it++;

    // :
    _it++;
    // label
    pLabel->name = _reader.readText(*_it++);
    // \n
    _it++;
    while (!match({TokenId::Colon}) && !match({TokenId::End}) && !match({TokenId::NewLine}))
    {
        auto pStatement = parseStatement();
        pLabel->statements.push_back(std::move(pStatement));
    }
    // skip empty lines
    while (match({TokenId::NewLine}))
        _it++;

    return pLabel;
}

std::unique_ptr<Ast::Statement> YackParser::parseStatement()
{
    auto pStatement = std::make_unique<Ast::Statement>();
    // expression
    auto pExp = parseExpression();
    pStatement->expression = std::move(pExp);
    // conditions
    while (match({TokenId::Condition}))
    {
        pStatement->conditions.push_back(parseCondition());
    }
    // skip \n
    _it++;
    return pStatement;
}

std::unique_ptr<Ast::Condition> YackParser::parseCondition()
{
    auto text = _reader.readText(*_it++);
    auto conditionText = text.substr(1, text.length() - 2);
    if (conditionText == "once")
    {
        return std::make_unique<Ast::OnceCondition>();
    }
    else if (conditionText == "showonce")
    {
        return std::make_unique<Ast::ShowOnceCondition>();
    }
    auto pCondition = std::make_unique<Ast::CodeCondition>();
    pCondition->code = conditionText;
    return pCondition;
}

std::unique_ptr<Ast::Expression> YackParser::parseExpression()
{
    if (match({TokenId::Identifier, TokenId::Colon, TokenId::String}))
        return parseSayExpression();
    if (match({TokenId::Identifier}))
        return parseInstructionExpression();
    if (match({TokenId::Goto}))
        return parseGotoExpression();
    if (match({TokenId::Number}))
        return parseChoiceExpression();
    if (match({TokenId::Code}))
        return parseCodeExpression();
    return nullptr;
}

bool YackParser::match(const std::initializer_list<TokenId> &ids)
{
    auto it = _it;
    for (auto id : ids)
    {
        if (it->id != id)
            return false;
        it++;
    }
    return true;
}

std::unique_ptr<Ast::Say> YackParser::parseSayExpression()
{
    auto actor = _reader.readText(*_it++);
    _it++;
    auto text = _reader.readText(*_it);
    _it++;
    auto pExp = std::make_unique<Ast::Say>();
    pExp->actor = actor;
    pExp->text = text.substr(1, text.length() - 2);
    return pExp;
}

std::unique_ptr<Ast::Expression> YackParser::parseInstructionExpression()
{
    // shutup
    // pause number
    // waitfor [actor]
    auto identifier = _reader.readText(*_it++);
    if (identifier == "shutup")
    {
        return std::make_unique<Ast::Shutup>();
    }
    else if (identifier == "pause")
    {
        auto time = std::atof(_reader.readText(*_it++).data());
        auto pExp = std::make_unique<Ast::Pause>();
        pExp->time = time;
        return pExp;
    }
    else if (identifier == "waitfor")
    {
        auto pExp = std::make_unique<Ast::WaitFor>();
        if (_it->id == TokenId::Identifier)
        {
            auto actor = _reader.readText(*_it++);
            pExp->actor = actor;
        }
        return pExp;
    }
    throw std::domain_error("Unknown instruction: " + identifier);
}

std::unique_ptr<Ast::Goto> YackParser::parseGotoExpression()
{
    _it++;
    auto name = _reader.readText(*_it++);
    auto pExp = std::make_unique<Ast::Goto>();
    pExp->name = name;
    return pExp;
}

std::unique_ptr<Ast::Code> YackParser::parseCodeExpression()
{
    auto code = _reader.readText(*_it++);
    auto pExp = std::make_unique<Ast::Code>();
    pExp->code = code.substr(1);
    return pExp;
}

std::unique_ptr<Ast::Choice> YackParser::parseChoiceExpression()
{
    auto number = std::atoi(_reader.readText(*_it).data());
    _it++;
    auto text = _reader.readText(*_it);
    _it++;
    auto pExp = std::make_unique<Ast::Choice>();
    pExp->number = number;
    pExp->text = text.substr(1, text.length() - 2);
    auto pGoto = parseGotoExpression();
    pExp->gotoExp = std::move(pGoto);
    return pExp;
}
} // namespace gg
