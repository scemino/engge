#include <cmath>
#include <algorithm>
#include "Math/PathFinding/Walkbox.hpp"
#include "../../System/_Util.hpp"
#include "_WalkboxDrawable.hpp"

namespace ng {
Walkbox::Walkbox() = default;

Walkbox::Walkbox(const Walkbox &w) = default;

Walkbox::Walkbox(std::vector<sf::Vector2i> polygon)
    : _polygon(std::move(polygon)), _isEnabled(true) {
}

Walkbox::~Walkbox() = default;

float Walkbox::distanceToSegment(sf::Vector2f p, sf::Vector2f v, sf::Vector2f w) {
  return sqrt(distanceToSegmentSquared(p, v, w));
}

float Walkbox::distanceToSegmentSquared(const sf::Vector2f &p, const sf::Vector2f &v, const sf::Vector2f &w) {
  float l2 = distanceSquared(v, w);
  if (l2 == 0)
    return distanceSquared(p, v);
  float t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
  if (t < 0)
    return distanceSquared(p, v);
  if (t > 1)
    return distanceSquared(p, w);
  return distanceSquared(p, sf::Vector2f(v.x + t * (w.x - v.x), v.y + t * (w.y - v.y)));
}

sf::Vector2f Walkbox::getClosestPointOnEdge(sf::Vector2f p3) const {
  int vi1 = -1;
  int vi2 = -1;
  float minDist = 100000;

  for (int i = 0; i < static_cast<int>(_polygon.size()); i++) {
    auto dist = distanceToSegment(p3, (sf::Vector2f) _polygon[i], (sf::Vector2f) _polygon[(i + 1) % _polygon.size()]);
    if (dist < minDist) {
      minDist = dist;
      vi1 = i;
      vi2 = (i + 1) % static_cast<int>(_polygon.size());
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

  if (u < 0)
    return sf::Vector2f(x1, y1);
  if (u > 1)
    return sf::Vector2f(x2, y2);
  return sf::Vector2f(xu, yu);
}

bool Walkbox::isVertexConcave(int vertex) const {
  auto current = _polygon[vertex];
  auto next = _polygon[(vertex + 1) % _polygon.size()];
  auto previous = _polygon[vertex == 0 ? _polygon.size() - 1 : vertex - 1];

  auto left = sf::Vector2f(current.x - previous.x, current.y - previous.y);
  auto right = sf::Vector2f(next.x - current.x, next.y - current.y);

  auto cross = (left.x * right.y) - (left.y * right.x);

  return cross < 0;
}

bool Walkbox::inside(const sf::Vector2f &position, bool toleranceOnOutside) const {
  sf::Vector2f point = position;
  const float epsilon = 1.0f;
  bool inside = false;

  // Must have 3 or more edges
  if (_polygon.size() < 3)
    return false;

  auto oldPoint = (sf::Vector2f) _polygon[_polygon.size() - 1];
  float oldSqDist = distanceSquared(oldPoint, point);

  for (auto nPoint : _polygon) {
    auto newPoint = (sf::Vector2f) nPoint;
    float newSqDist = distanceSquared(newPoint, point);

    if (oldSqDist + newSqDist + 2.0f * sqrt(oldSqDist * newSqDist) - distanceSquared(newPoint, oldPoint) < epsilon)
      return toleranceOnOutside;

    sf::Vector2f left;
    sf::Vector2f right;
    if (newPoint.x > oldPoint.x) {
      left = oldPoint;
      right = newPoint;
    } else {
      left = newPoint;
      right = oldPoint;
    }

    if (left.x < point.x && point.x <= right.x
        && (point.y - left.y) * (right.x - left.x) < (right.y - left.y) * (point.x - left.x))
      inside = !inside;

    oldPoint = newPoint;
    oldSqDist = newSqDist;
  }
  return inside;
}

std::ostream &operator<<(std::ostream &os, const Walkbox &walkbox) {
  os << "[ ";
  for (auto vertex : walkbox._polygon) {
    os << '(' << vertex.x << ',' << vertex.y << ") ";
  }
  os << ']';
  return os;
}

} // namespace ng
