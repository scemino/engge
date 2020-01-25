#pragma once

#include "SFML/Graphics.hpp"

namespace ng {
class Vector {
 public:
    Vector() = default;

    explicit Vector(const sf::Vector2f& v) {
        this->x = v.x;
        this->y = v.y;
    }

    explicit operator sf::Vector2f() const { return sf::Vector2f(x,y); }

    Vector& operator +=(const Vector& v){
        x += v.x;
        y += v.y;
        return *this;
    }

    friend Vector operator+(Vector lhs, const Vector& rhs) {
        lhs += rhs;
        return lhs;
    }

    friend bool operator==(Vector lhs, const Vector& rhs) {
        return lhs.x==rhs.x && lhs.y==rhs.y;
    }

    friend bool operator!=(Vector lhs, const Vector& rhs) {
        return lhs.x!=rhs.x || lhs.y!=rhs.y;
    }

    Vector& operator -=(const Vector& v){
        x -= v.x;
        y -= v.y;
        return *this;
    }

    friend Vector operator-(Vector lhs, const Vector& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend std::ostream &operator<<(std::ostream &os, const Vector &vector){
        os << "(" << vector.x << ',' << vector.y << ')';
        return os;
    }

 public:
    float x{0},y{0};
};
}

