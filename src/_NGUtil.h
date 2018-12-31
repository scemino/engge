#pragma once
#include <nlohmann/json.hpp>
#include <regex>
#include "Object.h"

namespace ng
{
static Facing _toFacing(UseDirection direction)
{
    switch (direction)
    {
    case UseDirection::Front:
        return Facing::FACE_FRONT;
    case UseDirection::Back:
        return Facing::FACE_BACK;
    case UseDirection::Left:
        return Facing::FACE_LEFT;
    case UseDirection::Right:
        return Facing::FACE_RIGHT;
    }
    throw std::logic_error("Invalid direction");
}

static sf::IntRect _toRect(const nlohmann::json &json)
{
    sf::IntRect rect;
    rect.left = json["x"].get<int>();
    rect.top = json["y"].get<int>();
    rect.width = json["w"].get<int>();
    rect.height = json["h"].get<int>();
    return rect;
}

static sf::Vector2i _toSize(const nlohmann::json &json)
{
    sf::Vector2i v;
    v.x = json["w"].get<int>();
    v.y = json["h"].get<int>();
    return v;
}

static UseDirection _toDirection(const std::string &text)
{
    if (strcmp(text.c_str(), "DIR_FRONT") == 0)
    {
        return UseDirection::Front;
    }
    if (strcmp(text.c_str(), "DIR_LEFT") == 0)
    {
        return UseDirection::Left;
    }
    if (strcmp(text.c_str(), "DIR_BACK") == 0)
    {
        return UseDirection::Back;
    }
    if (strcmp(text.c_str(), "DIR_RIGHT") == 0)
    {
        return UseDirection::Right;
    }
    return UseDirection::Front;
}

static sf::Vector2f _parsePos(const std::string &text)
{
    auto commaPos = text.find_first_of(',');
    auto x = atof(text.substr(1, commaPos - 1).c_str());
    auto y = atof(text.substr(commaPos + 1, text.length() - 1).c_str());
    return sf::Vector2f(x, y);
}

static sf::IntRect _parseRect(const std::string &text)
{
    auto re = std::regex("\\{\\{(\\-?\\d+),(\\-?\\d+)\\},\\{(\\-?\\d+),(\\-?\\d+)\\}\\}");
    std::smatch matches;
    std::regex_search(text, matches, re);
    auto left = std::atoi(matches[1].str().c_str());
    auto top = std::atoi(matches[2].str().c_str());
    auto right = std::atoi(matches[3].str().c_str());
    auto bottom = std::atoi(matches[4].str().c_str());
    return sf::IntRect(left, -top, right - left, -bottom + top);
}

static void _parsePolygon(const std::string &text, std::vector<sf::Vector2i> &vertices, int roomHeight)
{
    int i = 1;
    int endPos;
    do
    {
        auto commaPos = text.find_first_of(',', i);
        auto x = atoi(text.substr(i, commaPos - i).c_str());
        endPos = text.find_first_of('}', commaPos + 1);
        auto y = atoi(text.substr(commaPos + 1, endPos - commaPos - 1).c_str());
        i = endPos + 3;
        vertices.push_back(sf::Vector2i(x, roomHeight - y));
    } while (text.length() - 1 != endPos);
}

static sf::Color _toColor(SQInteger color)
{
    sf::Color c((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
    return c;
}

static sf::Color _fromRgb(SQInteger color)
{
    sf::Color c((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    return c;
}
} // namespace ng
