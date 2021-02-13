#include <algorithm>
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Parsers/YackTokenReader.hpp"
#include "engge/System/Logger.hpp"

namespace ng {
std::string Token::readToken() const {
  switch (id) {
  case TokenId::Assign:return "Assign";
  case TokenId::NewLine:return "NewLine";
  case TokenId::Colon:return "Colon";
  case TokenId::Code:return "Code";
  case TokenId::Comment:return "Comment";
  case TokenId::End:return "End";
  case TokenId::Goto:return "Goto";
  case TokenId::Identifier:return "Identifier";
  case TokenId::None:return "None";
  case TokenId::Number:return "Number";
  case TokenId::Condition:return "Condition";
  case TokenId::String:return "String";
  case TokenId::Whitespace:return "Whitespace";
  default:return "?";
  }
}

YackTokenReader::Iterator::Iterator(YackTokenReader &reader, std::streampos pos)
    : m_reader(reader),
      m_pos(pos) {
  operator++();
}

YackTokenReader::Iterator::Iterator(const Iterator &it)
    : m_reader(it.m_reader), m_pos(it.m_pos), m_token(it.m_token) {
}

YackTokenReader::Iterator &YackTokenReader::Iterator::operator++() {
  m_reader.m_stream.seek(m_pos);
  m_reader.readToken(m_token);
  m_pos = m_reader.m_stream.tell();
  return *this;
}

YackTokenReader::Iterator YackTokenReader::Iterator::operator++(int) {
  Iterator tmp(*this);
  operator++();
  return tmp;
}

Token &YackTokenReader::Iterator::operator*() {
  return m_token;
}

const Token &YackTokenReader::Iterator::operator*() const {
  return m_token;
}

Token *YackTokenReader::Iterator::operator->() {
  return &m_token;
}

void YackTokenReader::load(const std::string &path) {
  auto buffer = Locator<EngineSettings>::get().readBuffer(path);

#if 0
  std::ofstream o;
  o.open(path);
  o.write(buffer.data(), buffer.size());
  o.close();
#endif

  m_stream.setBuffer(buffer);
}

YackTokenReader::iterator YackTokenReader::begin() {
  return Iterator(*this, 0);
}
YackTokenReader::iterator YackTokenReader::end() {
  auto start = m_stream.tell();
  m_stream.seek(m_stream.getLength());
  auto pos = m_stream.tell();
  m_stream.seek(start);
  return Iterator(*this, pos);
}

YackTokenReader::YackTokenReader()
    : m_offset(-1) {
}

int YackTokenReader::getLine(const Token &token) const {
  auto previous = -1;
  auto previousVal = -1;
  for (auto &[key, val] : m_lines) {
    if ((previous < token.start) && (token.start < key)) {
      return val;
    }
    previousVal = val;
    previous = key;
  }
  return previousVal + 1;
}

bool YackTokenReader::readToken(Token &token) {
  std::streampos start = m_stream.tell();
  auto id = readTokenId();
  while (id == TokenId::Whitespace || id == TokenId::Comment || id == TokenId::NewLine || id == TokenId::None) {
    start = m_stream.tell();
    id = readTokenId();
  }
  std::streampos end = m_stream.tell();
  token.id = id;
  token.start = start;
  token.end = end;
  return true;
}

std::string YackTokenReader::readText(std::streampos pos, std::streamsize size) {
  std::string out;
  out.reserve(size);
  m_stream.seek(pos);
  char c;
  for (int i = 0; i < size; i++) {
    m_stream.read(&c, 1);
    out.append(&c, 1);
  }
  return out;
}

std::string YackTokenReader::readText(const Token &token) {
  return readText(token.start, token.end - token.start);
}

TokenId YackTokenReader::readTokenId() {
  char c;
  m_stream.read(&c, 1);
  if (m_stream.eof()) {
    return TokenId::End;
  }

  switch (c) {
  case '\0':return TokenId::End;
  case '\n':
    if (m_lines.find(m_stream.tell() - 1) == m_lines.end()) {
      m_lines[m_stream.tell() - 1] = m_offset == -1 ? 1 : m_lines[m_offset] + 1;
      m_offset = m_stream.tell() - 1;
    }
    return TokenId::NewLine;
  case '\t':
  case ' ':
    while (isspace(m_stream.peek()) && m_stream.peek() != '\n')
      m_stream.ignore();
    return TokenId::Whitespace;
  case '!':return readCode();
  case ':':return TokenId::Colon;
  case '$':return readDollar();
  case '[':return readCondition();
  case '=':return TokenId::Assign;
  case '\"':return readString();
  case '#':
  case ';':return readComment();
  default:
    if (c == '-' && m_stream.peek() == '>') {
      m_stream.ignore();
      return TokenId::Goto;
    }
    if (c == '-' || isdigit(c)) {
      return readNumber();
    } else if (isalpha(c)) {
      return readIdentifier(c);
    }
    error("unknown character: {}", c);
    return TokenId::None;
  }
}

TokenId YackTokenReader::readCode() {
  char c;
  char previousChar = '\0';
  while ((c = m_stream.peek()) != '\n' && c != '\0') {
    m_stream.ignore();
    if (previousChar == ' ' && c == '[' && m_stream.peek() != ' ') {
      m_stream.seek(m_stream.tell() - 1);
      return TokenId::Code;
    }
    previousChar = c;
  }
  return TokenId::Code;
}

TokenId YackTokenReader::readDollar() {
  char c;
  while ((c = m_stream.peek()) != '[' && c != ' ' && c != '\n' && c != '\0') {
    m_stream.ignore();
  }
  return TokenId::Dollar;
}

TokenId YackTokenReader::readCondition() {
  while (m_stream.peek() != ']') {
    m_stream.ignore();
  }
  m_stream.ignore();
  return TokenId::Condition;
}

TokenId YackTokenReader::readNumber() {
  while (isdigit(m_stream.peek())) {
    m_stream.ignore();
  }
  if (m_stream.peek() == '.') {
    m_stream.ignore();
  }
  while (isdigit(m_stream.peek())) {
    m_stream.ignore();
  }
  return TokenId::Number;
}

TokenId YackTokenReader::readComment() {
  m_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  m_stream.seek(m_stream.tell() - 1);
  return TokenId::Comment;
}

TokenId YackTokenReader::readString() {
  m_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
  return TokenId::String;
}

TokenId YackTokenReader::readIdentifier(char c) {
  std::string id;
  id.push_back(c);
  while (isalnum(m_stream.peek()) || m_stream.peek() == '_') {
    m_stream.read(&c, 1);
    id.push_back(c);
  }
  if (id == "waitwhile") {
    readCode();
    return TokenId::WaitWhile;
  }
  return TokenId::Identifier;
}
} // namespace ng
