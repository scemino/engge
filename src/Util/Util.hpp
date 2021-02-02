#pragma once
#include <optional>
#include <regex>
#include <ngf/IO/GGPackValue.h>
#include <ngf/Graphics/Sprite.h>
#include "engge/Parsers/GGPackBufferStream.hpp"
#include "engge/Entities/Actor/Costume.hpp"
#include "engge/Entities/Objects/Object.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/Text.hpp"

namespace ng {

struct CaseInsensitiveCompare {
  bool operator()(const std::string &a, const std::string &b) const noexcept {
#ifdef WIN32
    return _stricmp(a.c_str(), b.c_str()) == 0;
#else
    return ::strcasecmp(a.c_str(), b.c_str()) == 0;
#endif
  }
};

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

float distanceSquared(const glm::vec2 &vector1, const glm::vec2 &vector2);
float distance(const glm::vec2 &v1, const glm::vec2 &v2);

float length(const glm::vec2 &v);

Facing _toFacing(std::optional<UseDirection> direction);
UseDirection _toDirection(const std::string &text);
Facing getOppositeFacing(Facing facing);

ngf::irect _toRect(const ngf::GGPackValue &json);
glm::ivec2 _toSize(const ngf::GGPackValue &json);

glm::vec2 _parsePos(const std::string &text);
ngf::irect _parseRect(const std::string &text);
void _parsePolygon(const std::string &text, std::vector<glm::ivec2> &vertices);

ngf::Color _toColor(const std::string &color);
ngf::Color _toColor(SQInteger color);
ngf::Color _fromRgb(SQInteger color);

glm::vec2 toDefaultView(glm::ivec2 pos, glm::ivec2 fromSize);

InterpolationMethod toInterpolationMethod(SQInteger interpolation);

ngf::frect getGlobalBounds(const ng::Text &text);
ngf::frect getGlobalBounds(const ngf::Sprite &sprite);

ngf::GGPackValue toGGPackValue(HSQOBJECT obj);

} // namespace ng
