#pragma once
#include <vector>
#include "SFML/Graphics.hpp"

namespace gg
{
class Walkbox
{
  public:
    explicit Walkbox(const std::vector<sf::Vector2i> &polygon);
    ~Walkbox();

    void draw(sf::RenderWindow &window, sf::RenderStates states) const;
    void setName(const std::string &name) { _name = name; }
    const std::string &getName() const { return _name; }

  private:
    std::vector<sf::Vector2i> _polygon;
    std::string _name;
};
} // namespace gg
