#include <math.h>
#include <algorithm>
#include "Walkbox.h"
#include "_NGUtil.h"

namespace ng
{
Walkbox::Walkbox()
    : _color(sf::Color::Green)
{
}

Walkbox::Walkbox(const Walkbox &w)
    : _polygon(w._polygon), _name(w._name), _isEnabled(w._isEnabled), _color(w._color)
{
}

Walkbox::Walkbox(const std::vector<sf::Vector2i> &polygon)
    : _polygon(polygon), _isEnabled(true), _color(sf::Color::Green)
{
}

Walkbox::~Walkbox() = default;

float Walkbox::distanceToSegment(const sf::Vector2i &p, const sf::Vector2i &v, const sf::Vector2i &w)
{
    return sqrt(distanceToSegmentSquared(p, v, w));
}

float Walkbox::distanceToSegmentSquared(const sf::Vector2i &p, const sf::Vector2i &v, const sf::Vector2i &w)
{
    float l2 = distanceSquared(v, w);
    if (l2 == 0)
        return distanceSquared(p, v);
    float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
    if (t < 0)
        return distanceSquared(p, v);
    if (t > 1)
        return distanceSquared(p, w);
    return distanceSquared(p, sf::Vector2i(v.x + t * (w.x - v.x), v.y + t * (w.y - v.y)));
}

sf::Vector2i Walkbox::getClosestPointOnEdge(const sf::Vector2i &p3, float &mindist) const
{
    float tx = p3.x;
    float ty = p3.y;
    int vi1 = -1;
    int vi2 = -1;
    mindist = 100000;

    for (auto i = 0; i < _polygon.size(); i++)
    {
        auto dist = distanceToSegment(sf::Vector2i(tx, ty), _polygon[i], _polygon[(i + 1) % _polygon.size()]);
        if (dist < mindist)
        {
            mindist = dist;
            vi1 = i;
            vi2 = (i + 1) % _polygon.size();
        }
    }
    sf::Vector2i p1 = _polygon[vi1];
    sf::Vector2i p2 = _polygon[vi2];

    float x1 = p1.x;
    float y1 = p1.y;
    float x2 = p2.x;
    float y2 = p2.y;
    float x3 = p3.x;
    float y3 = p3.y;

    float u = (((x3 - x1) * (x2 - x1)) + ((y3 - y1) * (y2 - y1))) / (((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

    float xu = x1 + u * (x2 - x1);
    float yu = y1 + u * (y2 - y1);

    sf::Vector2i linevector;
    if (u < 0)
        linevector = sf::Vector2i(x1, y1);
    else if (u > 1)
        linevector = sf::Vector2i(x2, y2);
    else
        linevector = sf::Vector2i(xu, yu);

    return linevector;
}

void Walkbox::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto color = _isEnabled ? _color : sf::Color::Red;
    sf::VertexArray triangle(sf::LinesStrip, _polygon.size() + 1);
    for (int i = 0; i < _polygon.size(); ++i)
    {
        auto &vertex = _polygon[i];
        triangle[i].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[i].color = color;
    }
    {
        auto &vertex = _polygon[0];
        triangle[_polygon.size()].position = sf::Vector2f(vertex.x, vertex.y);
        triangle[_polygon.size()].color = color;
    }
    target.draw(triangle, states);
}

bool Walkbox::isVertexConcave(int vertex) const
{
    auto current = _polygon[vertex];
    auto next = _polygon[(vertex + 1) % _polygon.size()];
    auto previous = _polygon[vertex == 0 ? _polygon.size() - 1 : vertex - 1];

    auto left = sf::Vector2f(current.x - previous.x, current.y - previous.y);
    auto right = sf::Vector2f(next.x - current.x, next.y - current.y);

    auto cross = (left.x * right.y) - (left.y * right.x);

    return cross < 0;
}

bool Walkbox::inside(const sf::Vector2i &position, bool toleranceOnOutside) const
{
    sf::Vector2i point = position;
    const float epsilon = 0.5f;
    bool inside = false;

    // Must have 3 or more edges
    if (_polygon.size() < 3)
        return false;

    sf::Vector2i oldPoint = _polygon[_polygon.size() - 1];
    float oldSqDist = distanceSquared(oldPoint, point);

    for (auto i = 0; i < _polygon.size(); i++)
    {
        sf::Vector2i newPoint = _polygon[i];
        float newSqDist = distanceSquared(newPoint, point);

        if (oldSqDist + newSqDist + 2.0f * sqrt(oldSqDist * newSqDist) - distanceSquared(newPoint, oldPoint) < epsilon)
            return toleranceOnOutside;

        sf::Vector2i left;
        sf::Vector2i right;
        if (newPoint.x > oldPoint.x)
        {
            left = oldPoint;
            right = newPoint;
        }
        else
        {
            left = newPoint;
            right = oldPoint;
        }

        if (left.x < point.x && point.x <= right.x && (point.y - left.y) * (right.x - left.x) < (right.y - left.y) * (point.x - left.x))
            inside = !inside;

        oldPoint = newPoint;
        oldSqDist = newSqDist;
    }
    return inside;
}

std::ostream &operator<<(std::ostream &os, const Walkbox &walkbox)
{
    os << "[ ";
    for (auto vertex : walkbox._polygon)
    {
        os << '(' << vertex.x << ',' << vertex.y << ") ";
    }
    os << ']';
    return os;
}

} // namespace ng
