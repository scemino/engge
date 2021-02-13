#include <cassert>
#include "engge/Parsers/YackParser.hpp"

namespace ng {
Ast::Node::~Node() = default;
Ast::Expression::~Expression() = default;
Ast::Condition::Condition(int32_t line) : m_line(line) {}
Ast::Condition::~Condition() = default;
Ast::CodeCondition::CodeCondition(int32_t line) : Condition(line) {}
Ast::CodeCondition::~CodeCondition() = default;
Ast::OnceCondition::OnceCondition(int32_t line) : Condition(line) {}
Ast::OnceCondition::~OnceCondition() = default;
Ast::ShowOnceCondition::ShowOnceCondition(int32_t line) : Condition(line) {}
Ast::ShowOnceCondition::~ShowOnceCondition() = default;
Ast::OnceEverCondition::OnceEverCondition(int32_t line) : Condition(line) {}
Ast::OnceEverCondition::~OnceEverCondition() = default;
Ast::TempOnceCondition::TempOnceCondition(int32_t line) : Condition(line) {}
Ast::TempOnceCondition::~TempOnceCondition() = default;
Ast::Statement::~Statement() = default;
Ast::Goto::~Goto() = default;
Ast::Code::~Code() = default;
Ast::Choice::~Choice() = default;
Ast::Say::~Say() = default;
Ast::Pause::~Pause() = default;
Ast::Parrot::~Parrot() = default;
Ast::Dialog::~Dialog() = default;
Ast::Override::~Override() = default;
Ast::Shutup::~Shutup() = default;
Ast::AllowObjects::~AllowObjects() = default;
Ast::Limit::~Limit() = default;
Ast::WaitWhile::~WaitWhile() = default;
Ast::WaitFor::~WaitFor() = default;
Ast::Label::~Label() = default;
Ast::CompilationUnit::~CompilationUnit() = default;

std::ostream &operator<<(std::ostream &os, const Token &token) {
  return os << "[" << token.start << "," << token.end << "] " << token.readToken();
}

YackParser::YackParser(YackTokenReader &reader)
    : m_reader(reader), m_it(m_reader.begin()) {
}

std::unique_ptr<Ast::CompilationUnit> YackParser::parse() {
  auto pCu = std::make_unique<Ast::CompilationUnit>();
  while (!match({TokenId::End})) {
    pCu->labels.push_back(parseLabel());
  }
  return pCu;
}

std::unique_ptr<Ast::Label> YackParser::parseLabel() {
  auto pLabel = std::make_unique<Ast::Label>();

  // :
  m_it++;
  // label
  pLabel->name = m_reader.readText(*m_it++);
  do {
    if (match({TokenId::Colon}) || match({TokenId::End}))
      break;
    auto pStatement = parseStatement();
    pLabel->statements.push_back(std::move(pStatement));
  } while (true);

  return pLabel;
}

std::unique_ptr<Ast::Statement> YackParser::parseStatement() {
  auto pStatement = std::make_unique<Ast::Statement>();
  // expression
  auto pExp = parseExpression();
  pStatement->expression = std::move(pExp);
  // conditions
  while (match({TokenId::Condition})) {
    pStatement->conditions.push_back(parseCondition());
  }
  return pStatement;
}

std::unique_ptr<Ast::Condition> YackParser::parseCondition() {
  auto text = m_reader.readText(*m_it);
  auto conditionText = text.substr(1, text.length() - 2);
  auto line = m_reader.getLine(*m_it++);
  assert(line > 0);
  if (conditionText == "once") {
    return std::make_unique<Ast::OnceCondition>(line);
  } else if (conditionText == "showonce") {
    return std::make_unique<Ast::ShowOnceCondition>(line);
  } else if (conditionText == "onceever") {
    return std::make_unique<Ast::OnceEverCondition>(line);
  } else if (conditionText == "temponce") {
    return std::make_unique<Ast::TempOnceCondition>(line);
  }
  auto pCondition = std::make_unique<Ast::CodeCondition>(line);
  pCondition->code = conditionText;
  return pCondition;
}

std::unique_ptr<Ast::Expression> YackParser::parseExpression() {
  if (match({TokenId::Identifier, TokenId::Colon, TokenId::String}))
    return parseSayExpression();
  if (match({TokenId::WaitWhile}))
    return parseWaitWhileExpression();
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

bool YackParser::match(const std::initializer_list<TokenId> &ids) {
  auto it = m_it;
  for (auto id : ids) {
    if (it->id != id)
      return false;
    it++;
  }
  return true;
}

std::unique_ptr<Ast::Say> YackParser::parseSayExpression() {
  auto actor = m_reader.readText(*m_it++);
  m_it++;
  auto text = m_reader.readText(*m_it);
  m_it++;
  auto pExp = std::make_unique<Ast::Say>();
  pExp->actor = actor;
  pExp->text = text.substr(1, text.length() - 2);
  return pExp;
}

std::unique_ptr<Ast::Expression> YackParser::parseWaitWhileExpression() {
  auto waitwhile = m_reader.readText(*m_it++);
  auto code = waitwhile.substr(10);
  auto pExp = std::make_unique<Ast::WaitWhile>();
  pExp->condition = code;
  return pExp;
}

std::unique_ptr<Ast::Expression> YackParser::parseInstructionExpression() {
  auto identifier = m_reader.readText(*m_it++);
  if (identifier == "shutup") {
    return std::make_unique<Ast::Shutup>();
  } else if (identifier == "pause") {
    // pause number
    auto time = std::strtod(m_reader.readText(*m_it++).data(), nullptr);
    auto pExp = std::make_unique<Ast::Pause>();
    pExp->time = time;
    return pExp;
  } else if (identifier == "waitfor") {
    // waitfor [actor]
    auto pExp = std::make_unique<Ast::WaitFor>();
    if (m_it->id == TokenId::Identifier) {
      auto actor = m_reader.readText(*m_it++);
      pExp->actor = actor;
    }
    return pExp;
  } else if (identifier == "parrot") {
    // parrot [active]
    auto pExp = std::make_unique<Ast::Parrot>();
    if (m_it->id == TokenId::Identifier) {
      auto active = m_reader.readText(*m_it++);
      pExp->active = active == "yes";
    }
    return pExp;
  } else if (identifier == "dialog") {
    // dialog [actor]
    auto pExp = std::make_unique<Ast::Dialog>();
    if (m_it->id == TokenId::Identifier) {
      auto actor = m_reader.readText(*m_it++);
      pExp->actor = actor;
    }
    return pExp;
  } else if (identifier == "override") {
    // override [node]
    auto pExp = std::make_unique<Ast::Override>();
    if (m_it->id == TokenId::Identifier) {
      auto node = m_reader.readText(*m_it++);
      pExp->node = node;
    }
    return pExp;
  } else if (identifier == "allowobjects") {
    // allowobjects [allow]
    auto pExp = std::make_unique<Ast::AllowObjects>();
    if (m_it->id == TokenId::Identifier) {
      auto node = m_reader.readText(*m_it++);
      pExp->allow = node == "YES";
    }
    return pExp;
  } else if (identifier == "limit") {
    // limit [number]
    auto pExp = std::make_unique<Ast::Limit>();
    if (m_it->id == TokenId::Number) {
      auto node = m_reader.readText(*m_it++);
      pExp->max = std::strtol(node.c_str(), nullptr, 10);
    }
    return pExp;
  }
  throw std::domain_error("Unknown instruction: " + identifier);
}

std::unique_ptr<Ast::Goto> YackParser::parseGotoExpression() {
  m_it++;
  auto name = m_reader.readText(*m_it++);
  auto pExp = std::make_unique<Ast::Goto>();
  pExp->name = name;
  return pExp;
}

std::unique_ptr<Ast::Code> YackParser::parseCodeExpression() {
  auto code = m_reader.readText(*m_it++);
  auto pExp = std::make_unique<Ast::Code>();
  pExp->code = code.substr(1);
  return pExp;
}

std::unique_ptr<Ast::Choice> YackParser::parseChoiceExpression() {
  auto number = std::strtol(m_reader.readText(*m_it).data(), nullptr, 10);
  m_it++;
  std::string text;
  if (m_it->id == TokenId::Dollar) {
    text = m_reader.readText(*m_it);
  } else {
    text = m_reader.readText(*m_it);
    text = text.substr(1, text.length() - 2);
  }

  m_it++;
  auto pExp = std::make_unique<Ast::Choice>();
  pExp->number = number;
  pExp->text = text;
  auto pGoto = parseGotoExpression();
  pExp->gotoExp = std::move(pGoto);
  return pExp;
}
} // namespace ng
