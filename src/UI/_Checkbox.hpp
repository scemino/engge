#pragma once
#include <utility>
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "_ControlConstants.hpp"

namespace ng {
class _Checkbox : public sf::Drawable {
public:
  typedef std::function<void(bool)> Callback;

public:
  _Checkbox(int id, float y, bool enabled = true, bool checked = false, Callback callback = nullptr)
      : _id(id), _y(y), _isEnabled(enabled), _isChecked(checked), _callback(std::move(std::move(callback))) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    const FntFont &uiFontMedium = _pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    _text.setFont(uiFontMedium);
    _text.setString(_pEngine->getText(_id));
    auto textRect = _text.getLocalBounds();
    _text.setOrigin(sf::Vector2f(0, textRect.height));
    _text.setPosition(420.f, _y);
  }

  void setSpriteSheet(SpriteSheet *pSpriteSheet) {
    _pSpriteSheet = pSpriteSheet;
    auto checkedRect = pSpriteSheet->getRect("option_unchecked");
    _sprite.setPosition(820.f, _y);
    sf::Vector2f scale(Screen::Width / 320.f, Screen::Height / 180.f);
    _sprite.setScale(scale);
    _sprite.setOrigin(checkedRect.width / 2.f, checkedRect.height / 2.f);
    _sprite.setTexture(pSpriteSheet->getTexture());
    _sprite.setTextureRect(checkedRect);
  }

  void setChecked(bool checked) {
    if (_isChecked != checked) {
      _isChecked = checked;
      if (_callback) {
        _callback(_isChecked);
      }
    }
  }

  void update(sf::Vector2f pos) {
    auto textRect = _sprite.getGlobalBounds();

    sf::Color color;
    if (!_isEnabled) {
      color = _ControlConstants::DisabledColor;
    } else if (textRect.contains(pos)) {
      color = _ControlConstants::HoveColor;
      bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && _wasMouseDown && !isDown) {
        setChecked(!_isChecked);
      }
      _wasMouseDown = isDown;
    } else {
      color = _ControlConstants::NormalColor;
    }
    _sprite.setColor(color);
    _text.setFillColor(color);

    auto checkedRect =
        _isChecked ? _pSpriteSheet->getRect("option_checked") : _pSpriteSheet->getRect("option_unchecked");
    _sprite.setTextureRect(checkedRect);
  }

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    target.draw(_text, states);
    target.draw(_sprite, states);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  float _y{0};
  bool _isEnabled{true};
  bool _isChecked{false};
  bool _wasMouseDown{false};
  Callback _callback;
  Text _text;
  sf::Sprite _sprite;
  SpriteSheet *_pSpriteSheet{nullptr};
};
}
