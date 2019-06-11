#include "JsonTokenReader.h"

namespace ng
{
namespace Json
{
TokenReader::TokenReader() = default;

bool TokenReader::load(const std::string &path)
{
    std::ifstream is;
    is.open(path);

    if(!is.is_open()) 
        return false;

    is.seekg(-1, std::ios::end);
    auto size = is.tellg();
    is.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    is.read(buffer.data(), size);

#if 0
    std::ofstream o;
    o.open(path);
    o.write(buffer.data(), buffer.size());
    o.close();
#endif

    _stream.setBuffer(buffer);
    return true;
}

TokenId TokenReader::readString()
{
    while (isalpha(_stream.peek()))
    {
        _stream.ignore();
    }
    return TokenId::String;
}

TokenId TokenReader::readQuotedString()
{
    _stream.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
    return TokenId::String;
}

TokenId TokenReader::readNumber()
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

TokenId TokenReader::readTokenId()
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
        return TokenId::NewLine;
    case '\t':
    case ' ':
        while (isspace(_stream.peek()) && _stream.peek() != '\n')
            _stream.ignore();
        return TokenId::Whitespace;
    case ':':
        return TokenId::Colon;
    case '\"':
        return readQuotedString();
    default:
        if (c == '-' || isdigit(c))
        {
            return readNumber();
        }
        else if (isalpha(c))
        {
            return readString();
        }
        std::cerr << "unknown character: " << c << std::endl;
        return TokenId::None;
    }
}

std::string TokenReader::readText(std::streampos pos, std::streamsize size)
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

std::string TokenReader::readText(const Token &token)
{
    return readText(token.start, token.end - token.start);
}

bool TokenReader::readToken(Token &token)
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

TokenReader::Iterator::Iterator(TokenReader &reader, std::streampos pos)
    : _reader(reader),
      _pos(pos)
{
    operator++();
}

TokenReader::Iterator::Iterator(const Iterator &it)
    : _reader(it._reader), _pos(it._pos), _token(it._token)
{
}

TokenReader::Iterator &TokenReader::Iterator::operator++()
{
    _reader._stream.seek(_pos);
    _reader.readToken(_token);
    _pos = _reader._stream.tell();
    return *this;
}

const TokenReader::Iterator TokenReader::Iterator::operator++(int)
{
    Iterator tmp(*this);
    operator++();
    return tmp;
}

Token &TokenReader::Iterator::operator*()
{
    return _token;
}

const Token &TokenReader::Iterator::operator*() const
{
    return _token;
}

Token *TokenReader::Iterator::operator->()
{
    return &_token;
}

TokenReader::iterator TokenReader::begin()
{
    return Iterator(*this, 0);
}

TokenReader::iterator TokenReader::end()
{
    auto start = _stream.tell();
    _stream.seek(_stream.getLength());
    auto pos = _stream.tell();
    _stream.seek(start);
    return Iterator(*this, pos);
}

std::string Token::readToken() const
{
    switch (id)
    {
    case TokenId::NewLine:
        return "NewLine";
    case TokenId::Colon:
        return "Colon";
    case TokenId::End:
        return "End";
    case TokenId::None:
        return "None";
    case TokenId::Number:
        return "Number";
    case TokenId::String:
        return "String";
    case TokenId::Whitespace:
        return "Whitespace";
    default:
        return "?";
    }
}

std::ostream &operator<<(std::ostream &os, const Token &obj)
{
    os << obj.readToken();
    return os;
}

Parser::Parser() = default;

void Parser::parse(const std::string &path, GGPackValue &value)
{
    value.type = 2;

    ng::Json::TokenReader reader;
    if(!reader.load(path))
        return;

    auto it = reader.begin();
    while (it != reader.end())
    {
        auto token = *it;
        auto key = reader.readText(token);
        it++;it++;
        token = *it;
        auto text = reader.readText(token);
        GGPackValue packValue;
        switch (token.id)
        {
        case TokenId::Number:
            packValue.type = 6;
            packValue.double_value = std::atof(text.data());
            break;
        case TokenId::String:
            packValue.type = 4;
            packValue.string_value = text;
            break;
        default:
            break;
        }
        value.hash_value[key] = packValue;
        it++;
    }
}

} // namespace Json
} // namespace ng
