#pragma once
#include "SFML/Graphics.hpp"

namespace ng
{
class Engine;
class Camera
{
  public:
    Camera();
    virtual ~Camera();
    
    void at(const sf::Vector2f &at);
    void move(const sf::Vector2f &offset);
    void setBounds(const sf::IntRect &cameraBounds);
    void resetBounds();
    sf::Vector2f getAt() const;

    void setEngine(Engine *pEngine);

  private:
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
};
} // namespace ng