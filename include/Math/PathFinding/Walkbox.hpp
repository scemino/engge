#pragma once
#include <vector>
#include <list>
#include <iostream>
#include "SFML/Graphics.hpp"
#include "Entities/Entity.hpp"

namespace ng {
class Walkbox {
 public:
    Walkbox();
    Walkbox(const Walkbox &w);
    explicit Walkbox(std::vector<sf::Vector2i> polygon);
    ~Walkbox();

    void setName(const std::string &name) { _name = name; }
    [[nodiscard]] const std::string &getName() const { return _name; }
    void setEnabled(bool isEnabled) { _isEnabled = isEnabled; }
    [[nodiscard]] bool isEnabled() const { return _isEnabled; }

    [[nodiscard]] const std::vector<sf::Vector2i> &getVertices() const { return _polygon; }
    [[nodiscard]] const sf::Vector2i &getVertex(size_t index) const { return _polygon.at(index); }

    [[nodiscard]] bool inside(const sf::Vector2f &position, bool toleranceOnOutside = true) const;
    [[nodiscard]] bool isVertexConcave(int vertex) const;
    static float distanceToSegment(sf::Vector2f p, sf::Vector2f v, sf::Vector2f w);
    [[nodiscard]] sf::Vector2f getClosestPointOnEdge(sf::Vector2f p3) const;

    friend std::ostream &operator<<(std::ostream &os, const Walkbox &walkbox);

 private:
    static float distanceToSegmentSquared(const sf::Vector2f &p, const sf::Vector2f &v, const sf::Vector2f &w);

 private:
    std::vector<sf::Vector2i> _polygon;
    std::string _name;
    bool _isEnabled{true};
};

} // namespace ng
