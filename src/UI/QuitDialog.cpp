#include "_ControlConstants.hpp"
#include <engge/Graphics/FntFont.h>
#include "engge/Engine/Engine.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/UI/QuitDialog.hpp"
#include "_Button.hpp"
#include <utility>
#include <imgui.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Sprite.h>

namespace ng {
class _BackButton : public ngf::Drawable {
public:
  typedef std::function<void()> Callback;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    text.draw(target, states);
  }
public:
  _BackButton(int id, bool value, Callback callback, bool enabled = true)
      : _id(id), _isEnabled(enabled), _value(value), _callback(std::move(callback)) {
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;

    auto &uiFontLarge = _pEngine->getResourceManager().getFntFont("UIFontLarge.fnt");
    text.setFont(uiFontLarge);
    text.setWideString(Engine::getText(_id));
    auto textRect = text.getLocalBounds();
    auto originX = _value ? textRect.getWidth() : 0;
    auto x = _value ? -40.f : 40.f;
    text.getTransform().setOrigin({originX, textRect.getHeight() / 2.f});
    text.getTransform().setPosition({Screen::Width / 2.0f + x, 400.f});
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
        _callback();
      }
      _wasMouseDown = isDown;
    } else {
      color = _ControlConstants::NormalColor;
    }
    text.setColor(color);
  }

private:
  Engine *_pEngine{nullptr};
  int _id{0};
  bool _isEnabled{true};
  bool _value{false};
  bool _wasMouseDown{false};
  Callback _callback;
  ng::Text text;
};

struct QuitDialog::Impl {
  struct Ids {
    static constexpr int Yes = 99907;
    static constexpr int No = 99908;
    static constexpr int QuitText = 99909;
  };

  Engine *_pEngine{nullptr};
  SpriteSheet _saveLoadSheet;
  ng::Text _headingText;
  std::vector<_BackButton> _buttons;
  Callback _callback{nullptr};

  void setHeading(int id) {
    _headingText.setWideString(Engine::getText(id));
    auto textRect = _headingText.getLocalBounds();
    _headingText.getTransform().setPosition({(Screen::Width - textRect.getWidth()) / 2.f, 260.f});
  }

  void updateState() {
    _buttons.clear();

    setHeading(Ids::QuitText);

    _buttons.emplace_back(Ids::Yes, true, [this]() {
      if (_callback)
        _callback(true);
    });
    _buttons.emplace_back(Ids::No, false, [this]() {
      if (_callback)
        _callback(false);
    });

    for (auto &button : _buttons) {
      button.setEngine(_pEngine);
    }
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    if (!pEngine)
      return;

    ResourceManager &tm = pEngine->getResourceManager();
    _saveLoadSheet.setTextureManager(&tm);
    _saveLoadSheet.load("SaveLoadSheet");

    auto &headingFont = _pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    _headingText.setFont(headingFont);
    _headingText.setColor(ngf::Colors::White);

    updateState();
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates) {
    const auto view = target.getView();
    auto viewRect = ngf::frect::fromPositionSize({0, 0}, {320, 180});
    target.setView(ngf::View(viewRect));

    ngf::Color backColor{0, 0, 0, 128};
    ngf::RectangleShape fadeShape;
    fadeShape.setSize(viewRect.getSize());
    fadeShape.setColor(backColor);
    fadeShape.draw(target, {});

    // draw background
    auto viewCenter = glm::vec2(viewRect.getWidth() / 2, viewRect.getHeight() / 2);
    auto rect = _saveLoadSheet.getRect("error_dialog_small");
    ngf::Sprite sprite;
    sprite.getTransform().setPosition(viewCenter);
    sprite.setTexture(_saveLoadSheet.getTexture());
    sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2), static_cast<float>(rect.getHeight() / 2)});
    sprite.setTextureRect(rect);
    sprite.draw(target, {});

    viewRect = ngf::frect ::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
    target.setView(ngf::View(viewRect));

    // heading
    _headingText.draw(target, {});

    // controls
    for (auto &button : _buttons) {
      button.draw(target, {});
    }
    target.setView(view);
  }

  void update(const ngf::TimeSpan &) {
    auto pos = _pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
      ngf::View(ngf::frect::fromPositionSize({0,0},{Screen::Width,Screen::Height})));
    for (auto &button : _buttons) {
      button.update(pos);
    }
  }
};

QuitDialog::QuitDialog()
    : _pImpl(std::make_unique<Impl>()) {
}

QuitDialog::~QuitDialog() = default;

void QuitDialog::setEngine(Engine *pEngine) { _pImpl->setEngine(pEngine); }

void QuitDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  _pImpl->draw(target, states);
}

void QuitDialog::update(const ngf::TimeSpan &elapsed) {
  _pImpl->update(elapsed);
}

void QuitDialog::setCallback(Callback callback) {
  _pImpl->_callback = std::move(callback);
}

void QuitDialog::updateLanguage() {
  _pImpl->updateState();
}
}
