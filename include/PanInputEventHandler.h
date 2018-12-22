#pragma once
#include "SFML/Graphics.hpp"
#include "Game.h"
#include "Engine.h"

namespace ng
{
class PanInputEventHandler : public InputEventHandler
{
  public:
    PanInputEventHandler(Engine &engine, sf::RenderWindow &window)
        : _engine(engine),
          _window(window),
          _view(sf::FloatRect(0, 0, Screen::Width, Screen::Height)),
          _isMousePressed(false),
          _isKeyPressed(false)
    {
    }

    void run(sf::Event event) override
    {
        switch (event.type)
        {
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Button::Left)
            {
                _isMousePressed = true;
                _pos = sf::Mouse::getPosition();
            }
            break;
        case sf::Event::MouseButtonReleased:
            if (event.mouseButton.button == sf::Mouse::Button::Left)
            {
                _isMousePressed = false;
            }
            break;
        case sf::Event::MouseMoved:
        {
            if (!_isMousePressed || !_isKeyPressed)
                break;
            auto pos2 = sf::Mouse::getPosition(_window);
            auto delta = pos2 - _pos;
            if (abs(delta.x) < 50)
            {
                _engine.moveCamera(-(sf::Vector2f)delta);
            }
            _pos = pos2;
        }
        break;
        case sf::Event::KeyReleased:
        {
            if (event.key.code == sf::Keyboard::Key::Space)
            {
                _isKeyPressed = false;
            }
        }
        break;
        case sf::Event::KeyPressed:
        {
            if (event.key.code == sf::Keyboard::Key::Space)
            {
                _isKeyPressed = true;
            }
        }
        break;
        default:
            break;
        }
    }

  private:
    Engine &_engine;
    sf::RenderWindow &_window;
    sf::View _view;
    bool _isMousePressed, _isKeyPressed;
    sf::Vector2i _pos = sf::Mouse::getPosition();
};

class Engine;

class EngineShortcutsInputEventHandler : public InputEventHandler
{
  public:
    EngineShortcutsInputEventHandler(ng::Engine &engine, sf::RenderWindow &window)
        : _engine(engine), _window(window)
    {
    }

    void run(sf::Event event) override
    {
        switch (event.type)
        {
        case sf::Event::KeyPressed:
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            {
                _engine.getRoom().showDrawWalkboxes(!_engine.getRoom().areDrawWalkboxesVisible());
                break;
            }
            break;
        default:
            break;
        }
    }

  private:
    ng::Engine &_engine;
    sf::RenderWindow &_window;
};
} // namespace ng
