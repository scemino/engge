#pragma once
#include "SFML/Graphics.hpp"
#include "Engine/Camera.hpp"
#include "Game.hpp"
#include "Engine/Engine.hpp"
#include "Room/Room.hpp"
#include "Engine/SavegameSlot.hpp"
#include "Input/InputConstants.hpp"

namespace ng {
class DefaultInputEventHandler : public InputEventHandler {
public:
  DefaultInputEventHandler(Engine &engine, sf::RenderWindow &window)
      : _engine(engine),
        _window(window),
        _isMousePressed(false),
        _isKeyPressed(false) {
  }

  void run(sf::Event event) override {
    switch (event.type) {
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
        _engine.getCamera().move(-(sf::Vector2f) delta);
      }
      _pos = pos2;
    }
      break;
    case sf::Event::KeyReleased: {
      auto key = toKey(event.key.code);
      auto metaKey = toMetaKeys(event);
      if (key != InputConstants::NONE) {
        _engine.keyUp({metaKey, key});
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
        _engine.keyDown({metaKey, key});
      }
      if (event.key.code == sf::Keyboard::Key::Space) {
        _isKeyPressed = true;
      }
      break;
    }
    case sf::Event::JoystickButtonPressed: {
      auto key = toButtonKey(event.joystickButton);
      if (key != InputConstants::NONE) {
        _engine.keyDown(key);
      }
    }
      break;
    case sf::Event::JoystickButtonReleased: {
      auto key = toButtonKey(event.joystickButton);
      if (key != InputConstants::NONE) {
        _engine.keyUp(key);
      }
    }
      break;
    default:break;
    }
  }
  [[nodiscard]] MetaKeys toMetaKeys(const sf::Event &event) const {
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

private:
  Engine &_engine;
  sf::RenderWindow &_window;
  bool _isMousePressed, _isKeyPressed;
  sf::Vector2i _pos = sf::Mouse::getPosition();
};
} // namespace ng
