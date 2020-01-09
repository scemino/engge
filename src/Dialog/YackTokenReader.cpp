#include <algorithm>
#include "EngineSettings.h"
#include "Dialog/YackTokenReader.h"
#include "Logger.h"

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
    operator++();
}

YackTokenReader::Iterator::Iterator(const Iterator &it)
    : _reader(it._reader), _pos(it._pos), _token(it._token)
{
}

YackTokenReader::Iterator &YackTokenReader::Iterator::operator++()
{
    _reader._stream.seek(_pos);
    _reader.readToken(_token);
    _pos = _reader._stream.tell();
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
    return _token;
}

const Token &YackTokenReader::Iterator::operator*() const
{
  return _token;
}

Token *YackTokenReader::Iterator::operator->()
{
    return &_token;
}

void YackTokenReader::load(const std::string &path)
{
    std::vector<char> buffer;
    Locator<EngineSettings>::get().readEntry(path, buffer);

#if 0
    std::ofstream o;
    o.open(path);
    o.write(buffer.data(), buffer.size());
    o.close();
#endif

    _stream.setBuffer(buffer);
}

YackTokenReader::iterator YackTokenReader::begin()
{
    return Iterator(*this, 0);
}
YackTokenReader::iterator YackTokenReader::end()
{
    auto start = _stream.tell();
    _stream.seek(_stream.getLength());
    auto pos = _stream.tell();
    _stream.seek(start);
    return Iterator(*this, pos);
}

YackTokenReader::YackTokenReader()
    : _offset(-1)
{
}

bool YackTokenReader::readToken(Token &token)
{
    std::streampos start = _stream.tell();
    auto id = readTokenId();
    while (id == TokenId::Whitespace || id == TokenId::Comment || id == TokenId::NewLine || id == TokenId::None)
    {
        start = _stream.tell();
        id = readTokenId();
    }
    std::streampos end = _stream.tell();
    token.id = id;
    token.start = start;
    token.end = end;
    return true;
}

std::string YackTokenReader::readText(std::streampos pos, std::streamsize size)
{
    std::string out;
    out.reserve(size);
    _stream.seek(pos);
    char c;
    for (int i = 0; i < size; i++)
    {
        _stream.read(&c, 1);
        out.append(&c, 1);
    }
    return out;
}

std::string YackTokenReader::readText(const Token &token)
{
    return readText(token.start, token.end - token.start);
}

TokenId YackTokenReader::readTokenId()
{
    char c;
    _stream.read(&c, 1);
    if (_stream.eof())
    {
        return TokenId::End;
    }

    switch (c)
    {
    case '\0':
        return TokenId::End;
    case '\n':
        if (_lines.find(_stream.tell() - 1) == _lines.end())
        {
            _lines[_stream.tell() - 1] = _offset == -1 ? 1 : _lines[_offset] + 1;
            _offset = _stream.tell() - 1;
        }
        return TokenId::NewLine;
    case '\t':
    case ' ':
        while (isspace(_stream.peek()) && _stream.peek() != '\n')
            _stream.ignore();
        return TokenId::Whitespace;
    case '!':
        return readCode();
    case ':':
        return TokenId::Colon;
    case '$':
        return readDollar();
    case '[':
        return readCondition();
    case '=':
        return TokenId::Assign;
    case '\"':
        return readString();
    case '#':
    case ';':
        return readComment();
    default:
        if (c == '-' && _stream.peek() == '>')
        {
            _stream.ignore();
            return TokenId::Goto;
        }
        if (c == '-' || isdigit(c))
        {
            return readNumber();
        }
        else if (isalpha(c))
        {
            return readIdentifier(c);
        }
        error("unknown character: {}", c);
        return TokenId::None;
    }
}

TokenId YackTokenReader::readCode()
{
    char c;
    char previousChar = '\0';
    while ((c = _stream.peek()) != '\n' && c != '\0')
    {
        _stream.ignore();
        if(previousChar == ' ' && c == '[' && _stream.peek() != ' ')
        {
            _stream.seek(_stream.tell()-1);
            return TokenId::Code;
        }
        previousChar = c;
    }
    return TokenId::Code;
}

TokenId YackTokenReader::readDollar()
{
    char c;
    while ((c = _stream.peek()) != '[' && c != ' ' && c != '\n' && c != '\0')
    {
        _stream.ignore();
    }
    return TokenId::Dollar;
}

TokenId YackTokenReader::readCondition()
{
    while (_stream.peek() != ']')
    {
        _stream.ignore();
    }
    _stream.ignore();
    return TokenId::Condition;
}

TokenId YackTokenReader::readNumber()
{
    while (isdigit(_stream.peek()))
    {
        _stream.ignore();
    }
    if (_stream.peek() == '.')
    {
        _stream.ignore();
    }
    while (isdigit(_stream.peek()))
    {
        _stream.ignore();
    }
    return TokenId::Number;
}

TokenId YackTokenReader::readComment()
{
    _stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return TokenId::Comment;
}

TokenId YackTokenReader::readString()
{
    _stream.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
    return TokenId::String;
}

TokenId YackTokenReader::readIdentifier(char c)
{
    std::string id;
    id.push_back(c);
    while (isalnum(_stream.peek()) || _stream.peek() == '_')
    {
        _stream.read(&c, 1);
        id.push_back(c);
    }
    if (id == "waitwhile")
    {
        readCode();
        return TokenId::WaitWhile;
    }
    return TokenId::Identifier;
}
} // namespace ng
