#pragma once
#include <nlohmann/json.hpp>
#include <cmath>
#include <regex>
#include "Costume.h"
#include "Object.h"
#include "Walkbox.h"
#include "GGPack.h"

namespace ng
{
static bool getLine(GGPackBufferStream &input, std::string &line)
{
    char c;
    line.clear();
    do
    {
        input.read(&c, 1);
        if (c == 0 || c == '\n')
        {
            return input.tell() < input.getLength();
        }
        line.append(&c, 1);
    } while (true);
    return false;
}

static std::wstring towstring(const std::string &text)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(text.data(), text.data() + text.size());
}

static std::string tostring(const std::wstring &text)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(text);
}

static bool getLine(GGPackBufferStream &input, std::wstring &wline)
{
    std::string line;
    char c;
    do
    {
        input.read(&c, 1);
        if (c == 0 || c == '\n')
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            wline = converter.from_bytes(line.data(), line.data() + line.size());

            return input.tell() < input.getLength();
        }
        line.append(&c, 1);
    } while (true);

    return false;
}

static bool merge(const ng::Walkbox &w1, const ng::Walkbox &w2, std::vector<sf::Vector2i> &result)
{
    // I know this implementation is ugly :S
    sf::Vector2i v11, v12, v21, v22;
    bool hasMerged = false;
    for (int i1 = 0; i1 < w1.getVertices().size(); i1++)
    {
        v11 = w1.getVertex(i1);
        v12 = w1.getVertex((i1 + 1) % w1.getVertices().size());
        int iShared = -1;
        for (int i2 = w2.getVertices().size() - 1; i2 >= 0; i2--)
        {
            v21 = w2.getVertex(i2);
            auto i22 = i2;
            if (i22 == 0)
                i22 = w2.getVertices().size();
            v22 = w2.getVertex(i22 - 1);
            if (v11 == v21 && v12 == v22)
            {
                hasMerged = true;
                iShared = i2;
                break;
            }
        }
        if (iShared == -1)
        {
            result.push_back(v11);
        }
        else
        {
            for (int i2 = iShared; i2 < w2.getVertices().size() + iShared; i2++)
            {
                auto i2p = i2 % w2.getVertices().size();
                auto i2p2 = (i2p + 1) % w2.getVertices().size();
                v21 = w2.getVertex(i2p);
                v22 = w2.getVertex(i2p2);

                int iShared2 = -1;
                for (auto i1p = i1 + 2; i1p < w1.getVertices().size(); i1p++)
                {
                    v11 = w1.getVertex(i1p % w1.getVertices().size());
                    v12 = w1.getVertex((i1p + 1) % w1.getVertices().size());
                    if (v21 == v12 && v22 == v11)
                    {
                        iShared2 = i1p;
                        break;
                    }
                }
                if (iShared2 != -1)
                {
                    i1 = iShared2;
                    break;
                }
                else
                {
                    result.push_back(v21);
                }
            }
        }
    }
    return hasMerged;
}

static void merge(const std::vector<ng::Walkbox> &walkboxes, std::vector<Walkbox> &result)
{
    ng::Walkbox w;
    std::list<int> walkboxesProcessed;
    for (int i = 0; i < walkboxes.size(); i++)
    {
        if (walkboxes[i].isEnabled())
        {
            w = walkboxes[i];
            break;
        }
    }

    if (w.getVertices().empty())
        return;

    for (int i = 0; i < walkboxes.size(); i++)
    {
        if (std::find(walkboxesProcessed.begin(), walkboxesProcessed.end(), i) != walkboxesProcessed.end())
            continue;

        if (!walkboxes[i].isEnabled())
            continue;

        std::vector<sf::Vector2i> vertices;
        if (merge(w, walkboxes[i], vertices))
        {
            w = ng::Walkbox(vertices);
            walkboxesProcessed.push_back(i);
            i = -1;
        }
    }
    result.push_back(w);

    for (int i = 0; i < walkboxes.size(); i++)
    {
        if (std::find(walkboxesProcessed.begin(), walkboxesProcessed.end(), i) != walkboxesProcessed.end())
            continue;

        result.push_back(walkboxes[i]);
    }
}

static float distanceSquared(const sf::Vector2i &vector1, const sf::Vector2i &vector2)
{
    float dx = vector1.x - vector2.x;
    float dy = vector1.y - vector2.y;

    return dx * dx + dy * dy;
}

static float distance(const sf::Vector2i &v1, const sf::Vector2i &v2)
{
    return std::sqrt(distanceSquared(v1, v2));
}

static bool lineSegmentsCross(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Vector2f &c, const sf::Vector2f &d)
{
    auto denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));
    if (denominator == 0)
    {
        return false;
    }

    auto numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
    auto numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
    if (numerator1 == 0 || numerator2 == 0)
    {
        return false;
    }

    auto r = numerator1 / denominator;
    auto s = numerator2 / denominator;
    return (r > 0 && r < 1) && (s > 0 && s < 1);
}

static float length(const sf::Vector2i &v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static bool lineSegmentsCross(const sf::Vector2i &a, const sf::Vector2i &b, const sf::Vector2i &c, const sf::Vector2i &d)
{
    float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));
    if (denominator == 0)
    {
        return false;
    }

    float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
    float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
    if (numerator1 == 0 || numerator2 == 0)
    {
        return false;
    }

    float r = numerator1 / denominator;
    float s = numerator2 / denominator;

    return (r > 0 && r < 1) && (s > 0 && s < 1);
}

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
