#pragma once
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "_ControlConstants.hpp"

namespace ng {
class _Slider : public sf::Drawable {
public:
  typedef std::function<void(float)> Callback;

  _Slider(int id, float y, bool enabled = true, float value = 0.f, Callback callback = nullptr)
      : _id(id), _isEnabled(enabled), _y(y), _value(value), onValueChanged(callback) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
  }

  void setSpriteSheet(SpriteSheet *pSpriteSheet) {
    const FntFont &uiFontMedium = _pEngine->getTextureManager().getFntFont("UIFontMedium.fnt");
    _text.setFont(uiFontMedium);
    _text.setString(_pEngine->getText(_id));
    auto textRect = _text.getLocalBounds();
    _text.setPosition(Screen::Width / 2.f - textRect.width / 2.f, _y);

    auto sliderRect = pSpriteSheet->getRect("slider");
    auto handleRect = pSpriteSheet->getRect("slider_handle");
    sf::Vector2f scale(Screen::Width / 320.f, Screen::Height / 180.f);
    _sprite.setPosition(Screen::Width / 2.f, _y + textRect.height);
    _sprite.setScale(scale);
    _sprite.setOrigin(sliderRect.width / 2.f, 0);
    _sprite.setTexture(pSpriteSheet->getTexture());
    _sprite.setTextureRect(sliderRect);

    _min = Screen::Width / 2.f - (sliderRect.width * scale.x / 2.f);
    _max = Screen::Width / 2.f + (sliderRect.width * scale.x / 2.f);
    auto x = _min + _value * (_max - _min);
    _spriteHandle.setPosition(x, _y + textRect.height);
    _spriteHandle.setScale(scale);
    _spriteHandle.setOrigin(handleRect.width / 2.f, 0);
    _spriteHandle.setTexture(pSpriteSheet->getTexture());
    _spriteHandle.setTextureRect(handleRect);
  }

  void update(sf::Vector2f pos) {
    auto textRect = _sprite.getGlobalBounds();
    bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    if (!isDown) {
      _isDragging = false;
    }
    sf::Color color;
    if (!_isEnabled) {
      color = _ControlConstants::DisabledColor;
    } else if (textRect.contains(pos)) {
      color = _ControlConstants::HoveColor;
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && isDown) {
        _isDragging = true;
      }
    } else {
      color = _ControlConstants::NormalColor;
    }
    _sprite.setColor(color);
    _text.setFillColor(color);

    if (_isDragging) {
      auto x = std::clamp(pos.x, _min, _max);
      auto value = (x - _min) / (_max - _min);
      if (_value != value) {
        _value = value;
        if (onValueChanged) {
          onValueChanged.value()(value);
        }
      }
      _spriteHandle.setPosition(x, _spriteHandle.getPosition().y);
    }
  }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    target.draw(_text, states);
    target.draw(_sprite, states);
    target.draw(_spriteHandle, states);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  bool _isEnabled{true};
  float _y{0};
  float _min{0}, _max{0}, _value{0};
  bool _isDragging{false};
  sf::Sprite _sprite;
  sf::Sprite _spriteHandle;
  Text _text;
  std::optional<Callback> onValueChanged;
};
}
