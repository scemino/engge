#pragma once
#include <regex>
#include "Entities/Actor/Costume.hpp"
#include "Entities/Objects/Object.hpp"
#include "Math/PathFinding/Walkbox.hpp"
#include "Parsers/GGPack.hpp"
#include "../Math/Vector.hpp"
#include "Graphics/Screen.hpp"

namespace ng
{

struct CaseInsensitiveCompare
{
bool operator()(const std::string &a, const std::string &b) const noexcept
{
#ifdef WIN32
    return _stricmp(a.c_str(), b.c_str()) == 0;
#else
    return ::strcasecmp(a.c_str(), b.c_str()) == 0;
#endif
}
};

SQInteger int_rand(SQInteger min, SQInteger max);
float float_rand(float min, float max);

void replaceAll(std::string &text, const std::string &search, const std::string &replace);
void replaceAll(std::wstring &text, const std::wstring &search, const std::wstring &replace);

void removeFirstParenthesis(std::wstring &text);
bool startsWith(const std::string &str, const std::string &prefix);
bool endsWith(const std::string &str, const std::string &suffix);
void checkLanguage(std::string &str);

bool getLine(GGPackBufferStream &input, std::string &line);

std::wstring towstring(const std::string &text);
std::string tostring(const std::wstring &text);
std::string toUtf8(const std::wstring &text);
std::string str_toupper(std::string s);

bool getLine(GGPackBufferStream &input, std::wstring &wline);

float distanceSquared(const sf::Vector2f &vector1, const sf::Vector2f &vector2);
float distance(const sf::Vector2f &v1, const sf::Vector2f &v2);

bool lineSegmentsCross(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Vector2f &c, const sf::Vector2f &d);

float length(const sf::Vector2f &v);

Facing _toFacing(UseDirection direction);
UseDirection _toDirection(const std::string &text);
Facing getOppositeFacing(Facing facing);

sf::IntRect _toRect(const GGPackValue &json);
sf::Vector2i _toSize(const ng::GGPackValue &json);

sf::Vector2f _parsePos(const std::string &text);
sf::IntRect _parseRect(const std::string &text);
void _parsePolygon(const std::string &text, std::vector<sf::Vector2i> &vertices, int roomHeight);

sf::Color _toColor(const std::string& color);
sf::Color _toColor(SQInteger color);
sf::Color _fromRgb(SQInteger color);

sf::Vector2f toDefaultView(sf::Vector2i pos, sf::Vector2i fromSize);

} // namespace ng
