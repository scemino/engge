#include "engge/Game.hpp"
#include "engge/Graphics/Screen.hpp"
#include <imgui-SFML.h>
#include <imgui.h>
#include "engge/Engine/Preferences.hpp"
#include "engge/Engine/Camera.hpp"
#include "engge/System/Locator.hpp"
#include "Engine/_DebugFeatures.hpp"

namespace ng {
namespace {
static const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
static const std::string Title = "Engge";
static const sf::VideoMode VideoMode = sf::VideoMode(Screen::Width, Screen::Height);

static MetaKeys toMetaKeys(const sf::Event &event) {
  auto metaKey = (event.key.alt ? MetaKeys::Alt : MetaKeys::None) |
      (event.key.control ? MetaKeys::Control : MetaKeys::None) |
      (event.key.shift ? MetaKeys::Shift : MetaKeys::None) |
      (event.key.system ? MetaKeys::System : MetaKeys::None);
  return metaKey;
}

static InputConstants toButtonKey(sf::Event::JoystickButtonEvent event) {
  auto button = event.button;
  switch (button) {
  case 0:return InputConstants::BUTTON_A;
  case 1:return InputConstants::BUTTON_B;
  case 2:return InputConstants::BUTTON_X;
  case 3:return InputConstants::BUTTON_Y;
  case 4:return InputConstants::BUTTON_START;
  case 5:return InputConstants::BUTTON_BACK;
  default:return InputConstants::NONE;
  }
}

static InputConstants toKey(sf::Keyboard::Key key) {
  if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z) {
    return static_cast<InputConstants>(static_cast<int>(InputConstants::KEY_A) + (key - sf::Keyboard::Key::A));
  }
  if (key >= sf::Keyboard::Key::Num0 && key <= sf::Keyboard::Key::Num9) {
    return static_cast<InputConstants>(static_cast<int>(InputConstants::KEY_0) + (key - sf::Keyboard::Key::Num0));
  }
  if (key >= sf::Keyboard::Key::Numpad1 && key <= sf::Keyboard::Key::Numpad9) {
    return static_cast<InputConstants>(static_cast<int>(InputConstants::KEY_PAD1)
        + (key - sf::Keyboard::Key::Numpad1));
  }
  if (key >= sf::Keyboard::Key::F1 && key <= sf::Keyboard::Key::F12) {
    return static_cast<InputConstants>(static_cast<int>(InputConstants::KEY_F1) + (key - sf::Keyboard::Key::F1));
  }
  switch (key) {
  case sf::Keyboard::Key::Space:return InputConstants::KEY_SPACE;
  case sf::Keyboard::Key::Escape:return InputConstants::KEY_ESCAPE;
  case sf::Keyboard::Key::Left:return InputConstants::KEY_LEFT;
  case sf::Keyboard::Key::Right:return InputConstants::KEY_RIGHT;
  case sf::Keyboard::Key::Up:return InputConstants::KEY_UP;
  case sf::Keyboard::Key::Down:return InputConstants::KEY_DOWN;
  case sf::Keyboard::Key::Tab:return InputConstants::KEY_TAB;
  case sf::Keyboard::Key::Return:return InputConstants::KEY_RETURN;
  case sf::Keyboard::Key::Backspace:return InputConstants::KEY_BACKSPACE;
  default:return InputConstants::NONE;
  }
}
}

Game::Game() {
  auto style = getStyle();
  _window.create(VideoMode, Title, style);
  _window.setMouseCursorVisible(false);
  ImGui::SFML::Init(_window);

  Locator<Preferences>::get().subscribe([this](const std::string &name) {
    if (name == PreferenceNames::Fullscreen) {
      auto style = getStyle();
      auto view = _window.getView();
      _window.create(VideoMode, Title, style);
      _window.setView(view);
    }
  });
}

Game::~Game() { ImGui::SFML::Shutdown(); }

sf::Uint32 Game::getStyle() {
  return Locator<Preferences>::get().getUserPreference(PreferenceNames::Fullscreen, PreferenceDefaultValues::Fullscreen)
         ? sf::Style::Fullscreen : sf::Style::Default;
}

void Game::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
  _pEngine->setWindow(_window);
}

void Game::run() {
  _pEngine->run();
  sf::Clock clock;
  sf::Time timeSinceLastUpdate = sf::Time::Zero;
  while (_window.isOpen()) {
    sf::Time elapsed = clock.restart();
    timeSinceLastUpdate += elapsed;
    while (timeSinceLastUpdate > TimePerFrame) {
      timeSinceLastUpdate -= TimePerFrame;
      processEvents();
      update(TimePerFrame);
    }
    ImGui::SFML::Update(_window, elapsed);
    render();
  }
}

void Game::processEvents() {
  sf::Event event{};
  while (_window.pollEvent(event)) {
    ImGui::SFML::ProcessEvent(event);
    switch (event.type) {
    case sf::Event::Closed:_window.close();
      break;
    case sf::Event::MouseButtonPressed:
      if (event.mouseButton.button == sf::Mouse::Button::Left) {
        _isMousePressed = true;
        _pos = sf::Mouse::getPosition();
      }
      break;
    case sf::Event::MouseButtonReleased:
      if (event.mouseButton.button == sf::Mouse::Button::Left) {
        _isMousePressed = false;
      }
      break;
    case sf::Event::MouseMoved: {
      if (!_isMousePressed || !_isKeyPressed)
        break;
      auto pos2 = sf::Mouse::getPosition(_window);
      auto delta = pos2 - _pos;
      if (abs(delta.x) < 50) {
        _pEngine->getCamera().move(-(sf::Vector2f) delta);
      }
      _pos = pos2;
    }
      break;
    case sf::Event::KeyReleased: {
      auto key = toKey(event.key.code);
      auto metaKey = toMetaKeys(event);
      if (key != InputConstants::NONE) {
        _pEngine->keyUp({metaKey, key});
      }
      if (event.key.code == sf::Keyboard::Key::Space) {
        _isKeyPressed = false;
      }
    }
      break;
    case sf::Event::KeyPressed: {
      auto key = toKey(event.key.code);
      auto metaKey = toMetaKeys(event);
      if (key != InputConstants::NONE) {
        _pEngine->keyDown({metaKey, key});
      }
      if (event.key.code == sf::Keyboard::Key::Space) {
        _isKeyPressed = true;
      }
      break;
    }
    case sf::Event::JoystickButtonPressed: {
      auto key = toButtonKey(event.joystickButton);
      if (key != InputConstants::NONE) {
        _pEngine->keyDown(key);
      }
    }
      break;
    case sf::Event::JoystickButtonReleased: {
      auto key = toButtonKey(event.joystickButton);
      if (key != InputConstants::NONE) {
        _pEngine->keyUp(key);
      }
    }
      break;
    default:break;
    }
  }
}

void Game::update(const sf::Time &elapsed) {
  sf::Clock clock;
  _pEngine->update(sf::seconds(elapsed.asSeconds()));
  _DebugFeatures::_updateTime = clock.getElapsedTime();
}

void Game::render() {
  sf::Clock clock;
  _window.clear();
  _pEngine->draw(_window);
  ImGui::SFML::Render(_window);
  _window.display();
  _DebugFeatures::_renderTime = clock.getElapsedTime();
}
} // namespace ng
