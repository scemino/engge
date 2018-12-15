#include "Dialog/YackTokenReader.h"

namespace ng
{
std::string Token::readToken() const
{
    switch (id)
    {
    case TokenId::Assign:
        return "Assign";
    case TokenId::NewLine:
        return "NewLine";
    case TokenId::Colon:
        return "Colon";
    case TokenId::Code:
        return "Code";
    case TokenId::Comment:
        return "Comment";
    case TokenId::End:
        return "End";
    case TokenId::Goto:
        return "Goto";
    case TokenId::Identifier:
        return "Identifier";
    case TokenId::None:
        return "None";
    case TokenId::Number:
        return "Number";
    case TokenId::Condition:
        return "Condition";
    case TokenId::String:
        return "String";
    case TokenId::Whitespace:
        return "Whitespace";
    default:
        return "?";
    }
}

YackTokenReader::Iterator::Iterator(YackTokenReader &reader, std::streampos pos)
    : _reader(reader),
      _pos(pos)
{
}

YackTokenReader::Iterator &YackTokenReader::Iterator::operator++()
{
    _reader._is.seekg(_pos);
    _reader.readToken(_token);
    _pos = _reader._is.tellg();
    return *this;
}

YackTokenReader::Iterator YackTokenReader::Iterator::operator++(int)
{
    Iterator tmp(*this);
    operator++();
    return tmp;
}

Token &YackTokenReader::Iterator::operator*()
{
    _reader._is.seekg(_pos);
    _reader.readToken(_token);
    return _token;
}

Token *YackTokenReader::Iterator::operator->()
{
    _reader._is.seekg(_pos);
    _reader.readToken(_token);
    return &_token;
}

void YackTokenReader::load(const std::string &path)
{
    _is.open(path);
}

YackTokenReader::iterator YackTokenReader::begin()
{
    return Iterator(*this, 0);
}
YackTokenReader::iterator YackTokenReader::end()
{
    auto start = _is.tellg();
    _is.seekg(0, std::ios::end);
    auto pos = _is.tellg();
    _is.seekg(start, std::ios::beg);
    return Iterator(*this, pos);
}

bool YackTokenReader::readToken(Token &token)
{
    std::streampos start = _is.tellg();
    auto id = readTokenId();
    while (id == TokenId::Whitespace)
    {
        start = _is.tellg();
        id = readTokenId();
    }
    std::streampos end = _is.tellg();
    token.id = id;
    token.start = start;
    token.end = end;
    return true;
}

std::string YackTokenReader::readText(std::streampos pos, std::streamsize size)
{
    std::string out;
    out.reserve(size);
    _is.seekg(pos);
    std::copy_n(std::istreambuf_iterator(_is), size, std::back_inserter(out));
    return out;
}

std::string YackTokenReader::readText(const Token &token)
{
    return readText(token.start, token.end - token.start);
}

TokenId YackTokenReader::readTokenId()
{
    char c;
    _is.get(c);
    if (_is.eof())
    {
        _is.clear();
        return TokenId::End;
    }

    switch (c)
    {
    case '\0':
        return TokenId::End;
    case '\n':
        return TokenId::NewLine;
    case '\t':
    case ' ':
        while (isspace(_is.peek())&&_is.peek()!='\n')
            _is.ignore();
        return TokenId::Whitespace;
    case '!':
        return readCode();
    case ':':
        return TokenId::Colon;
    case '[':
        return readCondition();
    case '=':
        return TokenId::Assign;
    case '\"':
        return readString();
    case '#':
        return readComment();
    default:
        if (c == '-' && _is.peek() == '>')
        {
            _is.ignore();
            return TokenId::Goto;
        }
        if (c == '-' || isnumber(c))
        {
            return readNumber();
        }
        else if (isalpha(c))
        {
            return readIdentifier();
        }
        std::cerr << "unknown character: " << c << std::endl;
        return TokenId::None;
    }
}

TokenId YackTokenReader::readCode()
{
    char c;
    while ((c = _is.peek()) != '[' && c != '\n' && c != '\0')
    {
        _is.ignore();
    }
    return TokenId::Code;
}

TokenId YackTokenReader::readCondition()
{
    char c;
    while (_is.peek() != ']')
    {
        _is.ignore();
    }
    _is.ignore();
    return TokenId::Condition;
}

TokenId YackTokenReader::readNumber()
{
    char c;
    while (isnumber(_is.peek()))
    {
        _is.ignore();
    }
    if (_is.peek() == '.')
    {
        _is.ignore();
    }
    while (isnumber(_is.peek()))
    {
        _is.ignore();
    }
    return TokenId::Number;
}

TokenId YackTokenReader::readComment()
{
    _is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return TokenId::Comment;
}

TokenId YackTokenReader::readString()
{
    _is.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
    return TokenId::String;
}

TokenId YackTokenReader::readIdentifier()
{
    char c;
    while (isalnum(_is.peek()))
    {
        _is.ignore();
    }
    return TokenId::Identifier;
}
} // namespace ng
