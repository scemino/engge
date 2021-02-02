#pragma once
#include <imgui.h>
#include <engge/Graphics/FntFont.h>
#include <ngf/System/Mouse.h>
#include <engge/Graphics/Text.hpp>
#include "../Util/Util.hpp"
#include "ControlConstants.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Graphics/Screen.hpp"

namespace ng {
class Button final : public ngf::Drawable {
public:
  typedef std::function<void()> Callback;
  enum class Size { Large, Medium };

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    text.draw(target, states);
  }
public:
  Button(int id, float y, Callback callback, bool enabled = true, Size size = Size::Large)
      : _id(id), _isEnabled(enabled), _y(y), _callback(std::move(callback)), _size(size) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;

    const auto &uiFontLargeOrMedium =
        _pEngine->getResourceManager().getFntFont(_size == Size::Large ? "UIFontLarge.fnt" : "UIFontMedium.fnt");
    text.setFont(uiFontLargeOrMedium);
    text.setWideString(ng::Engine::getText(_id));
    auto textRect = text.getLocalBounds();
    text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
    text.getTransform().setPosition({Screen::Width / 2.f, _y});
  }

  void update(glm::vec2 pos) {
    auto textRect = ng::getGlobalBounds(text);
    ngf::Color color;
    if (!_isEnabled) {
      color = ControlConstants::DisabledColor;
    } else if (textRect.contains((glm::vec2) pos)) {
      color = ControlConstants::HoveColor;
      bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && _wasMouseDown && !isDown) {
        _callback();
      }
      _wasMouseDown = isDown;
    } else {
      color = ControlConstants::NormalColor;
    }
    text.setColor(color);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  bool _isEnabled{true};
  float _y{0};
  bool _wasMouseDown{false};
  Callback _callback;
  ng::Text text;
  Size _size{Size::Large};
};
}
