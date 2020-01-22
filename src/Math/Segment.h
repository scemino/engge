#pragma once

#include "SFML/Graphics.hpp"
#include "Vector.h"

namespace ng{
struct Segment {
 public:
    Segment() = default;
    Segment(const sf::Vector2f& start, const sf::Vector2f& end);

    void norm();

    [[nodiscard]] float dist(sf::Vector2f p) const;

    friend bool operator==(Segment lhs, const Segment& rhs) {
        return lhs.start==rhs.start && lhs.end==rhs.end;
    }

    friend bool operator!=(Segment lhs, const Segment& rhs) {
        return lhs.start!=rhs.start || lhs.end!=rhs.end;
    }

    Vector start;
    Vector end;
    float left{0}, right{0}, top{0}, bottom{0};
    float a{0},b{0},c{0};
};
}

