#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "GGPack.h"
#include "EngineSettings.h"

namespace ng
{
namespace Json
{
enum class TokenId
{
    None,
    NewLine,
    Number,
    Whitespace,
    Colon,
    String,
    Comment,
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

class TokenReader
{
public:
    class Iterator : public std::iterator<std::forward_iterator_tag, Token>
    {
    private:
        TokenReader &_reader;
        std::streampos _pos;
        Token _token;

    public:
        Iterator(TokenReader &reader, std::streampos pos);
        Iterator(const Iterator &it);
        Iterator &operator++();
        const Iterator operator++(int);

        bool operator==(const Iterator &rhs) const { return _pos == rhs._pos; }
        bool operator!=(const Iterator &rhs) const { return _pos != rhs._pos; }
        Token &operator*();
        const Token &operator*() const;
        Token *operator->();
    };

    typedef Iterator iterator;

public:
    TokenReader();

    bool load(const std::string &path);
    iterator begin();
    iterator end();
    bool readToken(Token &token);
    std::string readText(std::streampos pos, std::streamsize size);
    std::string readText(const Token &token);

private:
    TokenId readTokenId();
    TokenId readNumber();
    TokenId readString();
    TokenId readQuotedString();

private:
    GGPackBufferStream _stream;
};

class Parser
{
public:
    Parser();
    void parse(const std::string& path, GGPackValue &value);
};
} // namespace Json
} // namespace ng
