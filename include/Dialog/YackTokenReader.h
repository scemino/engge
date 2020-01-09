#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "GGPack.h"

namespace ng
{
enum class TokenId
{
  None,
  NewLine,
  Identifier,
  WaitWhile,
  Number,
  Whitespace,
  Colon,
  Condition,
  String,
  Assign,
  Comment,
  Goto,
  Code,
  Dollar,
  End
};

struct Token
{
  TokenId id;
  std::streampos start;
  std::streampos end;

  friend std::ostream &operator<<(std::ostream &os, const Token &obj);

private:
  std::string readToken() const;
};

class YackTokenReader
{
public:
  class Iterator : public std::iterator<std::forward_iterator_tag, Token>
  {
  private:
    YackTokenReader &_reader;
    std::streampos _pos;
    Token _token;

  public:
    Iterator(YackTokenReader &reader, std::streampos pos);
    Iterator(const Iterator &it);
    Iterator &operator++();
    Iterator operator++(int);

    bool operator==(const Iterator &rhs) const { return _pos == rhs._pos; }
    bool operator!=(const Iterator &rhs) const { return _pos != rhs._pos; }
    Token &operator*();
    const Token &operator*() const;
    Token *operator->();
  };

  typedef Iterator iterator;

public:
  YackTokenReader();

  void load(const std::string &path);
  iterator begin();
  iterator end();
  bool readToken(Token &token);
  std::string readText(std::streampos pos, std::streamsize size);
  std::string readText(const Token &token);

private:
  TokenId readTokenId();
  TokenId readCode();
  TokenId readCondition();
  TokenId readDollar();
  TokenId readNumber();
  TokenId readComment();
  TokenId readString();
  TokenId readIdentifier(char c);

private:
  GGPackBufferStream _stream;
  std::map<int,int> _lines;
  int _offset;
};
} // namespace ng
