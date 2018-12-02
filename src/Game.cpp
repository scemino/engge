#include "Game.h"
#include "Screen.h"

namespace gg
{
Game::Game(GGEngine &engine)
    : _engine(engine), _window(sf::VideoMode(320, 180), "Engge")
{
    _window.setSize(sf::Vector2u(1024, 768));
    _window.setFramerateLimit(60);
    _window.setMouseCursorVisible(false);
    _engine.setWindow(_window);
}

Game::~Game() = default;

void Game::run()
{
    sf::Clock clock;
    while (_window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        update(elapsed);
        processEvents();
        render();
    }
}

void Game::processEvents()
{
    sf::Event event{};
    while (_window.pollEvent(event))
    {
        for (auto &eh : _inputEventHandlers)
        {
            eh->run(event);
        }
        switch (event.type)
        {
        case sf::Event::Closed:
            _window.close();
            break;
        case sf::Event::KeyPressed:
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {
                _window.close();
            }
            break;
        default:
            break;
        }
    }
}

void Game::update(const sf::Time &elapsed)
{
    _engine.update(elapsed);
}

void Game::render()
{
    _window.clear();
    _engine.draw(_window);
    _window.display();
}
} // namespace gg
