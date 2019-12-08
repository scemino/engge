#pragma once
#include <vector>
#include <memory>
#include "SFML/Graphics.hpp"

namespace ng {
class _Path : public sf::Drawable {
 public:
    explicit _Path(std::vector<sf::Vector2f> path);

    [[nodiscard]] const std::vector<sf::Vector2f> &getPath() const { return _path; }

 private:
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

 private:
    std::vector<sf::Vector2f> _path;
};
}
