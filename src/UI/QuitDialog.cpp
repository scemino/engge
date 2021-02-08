#include "ControlConstants.hpp"
#include "Button.hpp"
#include <engge/Graphics/FntFont.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/UI/QuitDialog.hpp>
#include <utility>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/System/Mouse.h>
#include "Util/Util.hpp"

namespace ng {
class BackButton final : public Control {
public:
  typedef std::function<void()> Callback;

public:
  BackButton(int id, bool value, Callback callback, bool enabled = true)
      : Control(enabled), m_id(id), m_value(value), m_callback(std::move(callback)) {
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final {
    m_text.draw(target, states);
  }

  bool contains(glm::vec2 pos) const final {
    auto textRect = ng::getGlobalBounds(m_text);
    return textRect.contains((glm::vec2) pos);
  }

  void onEngineSet() final {
    auto &uiFontLarge = m_pEngine->getResourceManager().getFntFont("UIFontLarge.fnt");
    m_text.setFont(uiFontLarge);
    m_text.setWideString(Engine::getText(m_id));
    auto textRect = m_text.getLocalBounds();
    auto originX = m_value ? textRect.getWidth() : 0;
    auto x = m_value ? -40.f : 40.f;
    m_text.getTransform().setOrigin({originX, textRect.getHeight() / 2.f});
    m_text.getTransform().setPosition({Screen::Width / 2.0f + x, 400.f});
  }

  void onStateChanged() final {
    ngf::Color color;
    switch (m_state) {
    case ControlState::Disabled:color = ControlConstants::DisabledColor;
      break;
    case ControlState::None:color = ControlConstants::NormalColor;
      break;
    case ControlState::Hover:color = ControlConstants::HoverColor;
      break;
    }
    m_text.setColor(color);
  }

  void onClick() final {
    if (m_callback) {
      m_callback();
    }
  }

private:
  int m_id{0};
  bool m_value{false};
  Callback m_callback;
  ng::Text m_text;
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
  std::vector<BackButton> _buttons;
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
    sprite.setTexture(*_saveLoadSheet.getTexture());
    sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2),
                                     static_cast<float>(rect.getHeight() / 2)});
    sprite.setTextureRect(rect);
    sprite.draw(target, {});

    viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
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
                                                                               ngf::View(ngf::frect::fromPositionSize({0,
                                                                                                                       0},
                                                                                                                      {Screen::Width,
                                                                                                                       Screen::Height})));
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
