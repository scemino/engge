#pragma once
#include <nlohmann/json.hpp>
#include <regex>
#include "Costume.h"
#include "Object.h"
#include "Walkbox.h"
#include "GGPack.h"

namespace ng
{
SQInteger int_rand(SQInteger min, SQInteger max);
float float_rand(float min, float max);

void replaceAll(std::string &text, const std::string &search, const std::string &replace);
void replaceAll(std::wstring &text, const std::wstring &search, const std::wstring &replace);

void removeFirstParenthesis(std::wstring &text);

bool getLine(GGPackBufferStream &input, std::string &line);

std::wstring towstring(const std::string &text);
std::string tostring(const std::wstring &text);

bool getLine(GGPackBufferStream &input, std::wstring &wline);

bool merge(const ng::Walkbox &w1, const ng::Walkbox &w2, std::vector<sf::Vector2i> &result);
void merge(const std::vector<ng::Walkbox> &walkboxes, std::vector<Walkbox> &result);

float distanceSquared(const sf::Vector2i &vector1, const sf::Vector2i &vector2);
float distance(const sf::Vector2i &v1, const sf::Vector2i &v2);

bool lineSegmentsCross(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Vector2f &c, const sf::Vector2f &d);
bool lineSegmentsCross(const sf::Vector2i &a, const sf::Vector2i &b, const sf::Vector2i &c, const sf::Vector2i &d);

float length(const sf::Vector2i &v);

Facing _toFacing(UseDirection direction);
UseDirection _toDirection(const std::string &text);

sf::IntRect _toRect(const nlohmann::json &json);
sf::Vector2i _toSize(const nlohmann::json &json);

sf::Vector2f _parsePos(const std::string &text);
sf::IntRect _parseRect(const std::string &text);
void _parsePolygon(const std::string &text, std::vector<sf::Vector2i> &vertices, int roomHeight);

sf::Color _toColor(std::string color);
sf::Color _toColor(SQInteger color);
sf::Color _fromRgb(SQInteger color);
} // namespace ng
