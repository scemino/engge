#pragma once
#include <imgui.h>
#include "_ControlConstants.hpp"
#include <engge/Graphics/FntFont.h>

namespace ng {
class _SwitchButton : public ngf::Drawable {
public:
  typedef std::function<void(int)> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    text.draw(target, states);
  }
public:
  _SwitchButton(std::initializer_list<int> ids,
                float y,
                bool enabled = true,
                int index = 0,
                Callback callback = nullptr)
      : _ids(ids), _index(index), _isEnabled(enabled), _y(y), _callback(std::move(callback)) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;

    auto &uiFontMedium = _pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    text.setFont(uiFontMedium);
    text.setWideString(Engine::getText(_ids[_index]));
    auto textRect = text.getLocalBounds();
    text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
    text.getTransform().setPosition({Screen::Width / 2.f, _y});
  }

  void update(glm::vec2 pos) {
    auto textRect = ng::getGlobalBounds(text);

    ngf::Color color;
    if (!_isEnabled) {
      color = _ControlConstants::DisabledColor;
    } else if (textRect.contains(pos)) {
      color = _ControlConstants::HoveColor;
      bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
      ImGuiIO &io = ImGui::GetIO();
      if (!io.WantCaptureMouse && _wasMouseDown && !isDown) {
        _index = (_index + 1) % static_cast<int>(_ids.size());
        text.setWideString(Engine::getText(_ids[_index]));
        if (_callback) {
          _callback(_index);
        }
        textRect = text.getLocalBounds();
        text.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
      }
      _wasMouseDown = isDown;
    } else {
      color = _ControlConstants::NormalColor;
    }
    text.setColor(color);
  }

private:
  Engine *_pEngine{nullptr};
  std::vector<int> _ids;
  int _index{0};
  bool _isEnabled{true};
  float _y{0};
  bool _wasMouseDown{false};
  Callback _callback;
  ng::Text text;
};
}
