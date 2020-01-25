#include <cmath>
#include "Segment.hpp"

namespace ng{

const double EPS = 1E-9;

Segment::Segment(const sf::Vector2f& start, const sf::Vector2f& end){
    this->start = Vector(start);
    this->end = Vector(end);
    this->left = fmin(start.x, end.x);
    this->right = fmax(start.x, end.x);
    this->top = fmin(start.y, end.y);
    this->bottom = fmax(start.y, end.y);
    a = start.y - end.y;
    b = end.x - start.x;
    c = -a * start.x - b * start.y;
    norm();
}

void Segment::norm()
{
    float z = sqrtf(a * a + b * b);
    if (fabs(z) > EPS)
        a /= z, b /= z, c /= z;
}

float Segment::dist(sf::Vector2f p) const { return a * p.x + b * p.y + c; }
}
