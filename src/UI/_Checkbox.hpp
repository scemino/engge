#pragma once
#include <utility>
#include <imgui.h>
#include "_ControlConstants.hpp"
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Sprite.h>

namespace ng {
class _Checkbox : public ngf::Drawable {
public:
  typedef std::function<void(bool)> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    _text.draw(target, states);
    _sprite.draw(target, states);
  }
public:
  _Checkbox(int id, float y, bool enabled = true, bool checked = false, Callback callback = nullptr)
      : _id(id), _y(y), _isEnabled(enabled), _isChecked(checked), _callback(std::move(std::move(callback))) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    auto &uiFontMedium = _pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    _text.setFont(uiFontMedium);
    _text.setWideString(_pEngine->getText(_id));
    auto textRect = _text.getLocalBounds();
    _text.getTransform().setOrigin(glm::vec2(0, textRect.getHeight()));
    _text.getTransform().setPosition({420.f, _y});
    _text.setAlignment(ngf::Alignment::Left);
    _text.setMaxWidth(600);
    _text.setAnchor(ngf::Anchor::CenterLeft);
  }

  void setSpriteSheet(SpriteSheet *pSpriteSheet) {
    _pSpriteSheet = pSpriteSheet;
    auto checkedRect = pSpriteSheet->getRect("option_unchecked");
    _sprite.getTransform().setPosition({820.f, _y});
    glm::vec2 scale(Screen::Width / 320.f, Screen::Height / 180.f);
    _sprite.getTransform().setScale(scale);
    _sprite.getTransform().setOrigin({checkedRect.getWidth() / 2.f, checkedRect.getHeight() / 2.f});
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

  void update(glm::vec2 pos) {
    auto textRect = ng::getGlobalBounds(_sprite);

    ngf::Color color;
    if (!_isEnabled) {
      color = _ControlConstants::DisabledColor;
    } else if (textRect.contains(pos)) {
      color = _ControlConstants::HoveColor;
      bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && _wasMouseDown && !isDown) {
        setChecked(!_isChecked);
      }
      _wasMouseDown = isDown;
    } else {
      color = _ControlConstants::NormalColor;
    }
    _sprite.setColor(color);
    _text.setColor(color);

    auto checkedRect =
        _isChecked ? _pSpriteSheet->getRect("option_checked") : _pSpriteSheet->getRect("option_unchecked");
    _sprite.setTextureRect(checkedRect);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  float _y{0};
  bool _isEnabled{true};
  bool _isChecked{false};
  bool _wasMouseDown{false};
  Callback _callback;
  ngf::Text _text;
  ngf::Sprite _sprite;
  SpriteSheet *_pSpriteSheet{nullptr};
};
}
