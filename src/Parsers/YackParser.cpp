#include "Parsers/YackParser.hpp"

namespace ng {
Ast::Node::~Node() = default;
Ast::Expression::~Expression() = default;
Ast::Condition::~Condition() = default;
Ast::CodeCondition::~CodeCondition() = default;
Ast::OnceCondition::~OnceCondition() = default;
Ast::ShowOnceCondition::~ShowOnceCondition() = default;
Ast::OnceEverCondition::~OnceEverCondition() = default;
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
    : _reader(reader), _it(_reader.begin()) {
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
  _it++;
  // label
  pLabel->name = _reader.readText(*_it++);
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
  auto text = _reader.readText(*_it++);
  auto conditionText = text.substr(1, text.length() - 2);
  if (conditionText == "once") {
    return std::make_unique<Ast::OnceCondition>();
  } else if (conditionText == "showonce") {
    return std::make_unique<Ast::ShowOnceCondition>();
  } else if (conditionText == "onceever") {
    return std::make_unique<Ast::OnceEverCondition>();
  } else if (conditionText == "temponce") {
    return std::make_unique<Ast::TempOnceCondition>();
  }
  auto pCondition = std::make_unique<Ast::CodeCondition>();
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
  auto it = _it;
  for (auto id : ids) {
    if (it->id != id)
      return false;
    it++;
  }
  return true;
}

std::unique_ptr<Ast::Say> YackParser::parseSayExpression() {
  auto actor = _reader.readText(*_it++);
  _it++;
  auto text = _reader.readText(*_it);
  _it++;
  auto pExp = std::make_unique<Ast::Say>();
  pExp->actor = actor;
  pExp->text = text.substr(1, text.length() - 2);
  return pExp;
}

std::unique_ptr<Ast::Expression> YackParser::parseWaitWhileExpression() {
  auto waitwhile = _reader.readText(*_it++);
  auto code = waitwhile.substr(10);
  auto pExp = std::make_unique<Ast::WaitWhile>();
  pExp->condition = code;
  return pExp;
}

std::unique_ptr<Ast::Expression> YackParser::parseInstructionExpression() {
  auto identifier = _reader.readText(*_it++);
  if (identifier == "shutup") {
    return std::make_unique<Ast::Shutup>();
  } else if (identifier == "pause") {
    // pause number
    auto time = std::strtod(_reader.readText(*_it++).data(), nullptr);
    auto pExp = std::make_unique<Ast::Pause>();
    pExp->time = time;
    return pExp;
  } else if (identifier == "waitfor") {
    // waitfor [actor]
    auto pExp = std::make_unique<Ast::WaitFor>();
    if (_it->id == TokenId::Identifier) {
      auto actor = _reader.readText(*_it++);
      pExp->actor = actor;
    }
    return pExp;
  } else if (identifier == "parrot") {
    // parrot [active]
    auto pExp = std::make_unique<Ast::Parrot>();
    if (_it->id == TokenId::Identifier) {
      auto active = _reader.readText(*_it++);
      pExp->active = active == "yes";
    }
    return pExp;
  } else if (identifier == "dialog") {
    // dialog [actor]
    auto pExp = std::make_unique<Ast::Dialog>();
    if (_it->id == TokenId::Identifier) {
      auto actor = _reader.readText(*_it++);
      pExp->actor = actor;
    }
    return pExp;
  } else if (identifier == "override") {
    // override [node]
    auto pExp = std::make_unique<Ast::Override>();
    if (_it->id == TokenId::Identifier) {
      auto node = _reader.readText(*_it++);
      pExp->node = node;
    }
    return pExp;
  } else if (identifier == "allowobjects") {
    // allowobjects [allow]
    auto pExp = std::make_unique<Ast::AllowObjects>();
    if (_it->id == TokenId::Identifier) {
      auto node = _reader.readText(*_it++);
      pExp->allow = node == "YES";
    }
    return pExp;
  } else if (identifier == "limit") {
    // limit [number]
    auto pExp = std::make_unique<Ast::Limit>();
    if (_it->id == TokenId::Number) {
      auto node = _reader.readText(*_it++);
      pExp->max = std::strtol(node.c_str(), nullptr, 10);
    }
    return pExp;
  }
  throw std::domain_error("Unknown instruction: " + identifier);
}

std::unique_ptr<Ast::Goto> YackParser::parseGotoExpression() {
  _it++;
  auto name = _reader.readText(*_it++);
  auto pExp = std::make_unique<Ast::Goto>();
  pExp->name = name;
  return pExp;
}

std::unique_ptr<Ast::Code> YackParser::parseCodeExpression() {
  auto code = _reader.readText(*_it++);
  auto pExp = std::make_unique<Ast::Code>();
  pExp->code = code.substr(1);
  return pExp;
}

std::unique_ptr<Ast::Choice> YackParser::parseChoiceExpression() {
  auto number = std::strtol(_reader.readText(*_it).data(), nullptr, 10);
  _it++;
  std::string text;
  if (_it->id == TokenId::Dollar) {
    text = _reader.readText(*_it);
  } else {
    text = _reader.readText(*_it);
    text = text.substr(1, text.length() - 2);
  }

  _it++;
  auto pExp = std::make_unique<Ast::Choice>();
  pExp->number = number;
  pExp->text = text;
  auto pGoto = parseGotoExpression();
  pExp->gotoExp = std::move(pGoto);
  return pExp;
}
} // namespace ng
