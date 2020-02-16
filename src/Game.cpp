#include "Game.hpp"
#include "Graphics/Screen.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

namespace ng
{
const sf::Time Game::TimePerFrame = sf::seconds(1.f/60.f);

Game::Game() : _window(sf::VideoMode(Screen::Width, Screen::Height), "Engge")
{
    _window.setSize(sf::Vector2u(Screen::Width, Screen::Height));
    _window.setMouseCursorVisible(false);
    ImGui::SFML::Init(_window);
}

Game::~Game() { ImGui::SFML::Shutdown(); }

void Game::setEngine(Engine *pEngine)
{
    _pEngine = pEngine;
    _pEngine->setWindow(_window);
}

void Game::run()
{
    _pEngine->run();
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (_window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        timeSinceLastUpdate += elapsed;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }
        ImGui::SFML::Update(_window, elapsed);
        render();
    }
}

void Game::processEvents()
{
    sf::Event event{};
    while (_window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);
        for (auto &eh : _inputEventHandlers)
        {
            eh->run(event);
        }
        switch (event.type)
        {
            case sf::Event::Closed:
                _window.close();
                break;
            default:
                break;
        }
    }
}

void Game::update(const sf::Time &elapsed)
{
    _pEngine->update(sf::seconds(elapsed.asSeconds()));
}

void Game::render()
{
    _window.clear();
    _pEngine->draw(_window);
    ImGui::SFML::Render(_window);
    _window.display();
}
} // namespace ng
