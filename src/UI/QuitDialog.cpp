#include "ControlConstants.hpp"
#include "Button.hpp"
#include <engge/EnggeApplication.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/UI/QuitDialog.hpp>
#include <utility>
#include <ngf/Graphics/FntFont.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/System/Mouse.h>
#include "Util/Util.hpp"

namespace ng {
class BackButton final : public Control {
public:
  using Callback = std::function<void()>;

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
    m_text.setFont(m_pEngine->getResourceManager().getFntFont("UIFontLarge.fnt"));
    m_text.setWideString(Engine::getText(m_id));
    auto textRect = m_text.getLocalBounds();
    auto originX = m_value ? textRect.getWidth() : 0;
    auto x = m_value ? -60.f : 60.f;
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

  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final {
    Control::update(elapsed, pos);
    auto x = m_value ? -60.f : 60.f;
    m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.0f + x, 400.f});
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

  Engine *m_pEngine{nullptr};
  SpriteSheet m_saveLoadSheet;
  ng::Text m_headingText;
  std::vector<BackButton> m_buttons;
  Callback m_callback{nullptr};

  void setHeading(int id) {
    m_headingText.setWideString(Engine::getText(id));
    auto textRect = m_headingText.getLocalBounds();
    m_headingText.getTransform().setPosition({(Screen::Width - textRect.getWidth()) / 2.f, 260.f});
  }

  void updateState() {
    m_buttons.clear();

    setHeading(Ids::QuitText);

    m_buttons.emplace_back(Ids::Yes, true, [this]() {
      if (m_callback)
        m_callback(true);
    });
    m_buttons.emplace_back(Ids::No, false, [this]() {
      if (m_callback)
        m_callback(false);
    });

    for (auto &button : m_buttons) {
      button.setEngine(m_pEngine);
    }
  }

  void setEngine(Engine *pEngine) {
    m_pEngine = pEngine;
    if (!pEngine)
      return;

    m_saveLoadSheet.setTextureManager(&pEngine->getResourceManager());
    m_saveLoadSheet.load("SaveLoadSheet");

    auto &headingFont = m_pEngine->getResourceManager().getFntFont("UIFontMedium.fnt");
    m_headingText.setFont(headingFont);
    m_headingText.setColor(ngf::Colors::White);

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
    auto rect = m_saveLoadSheet.getRect("error_dialog_small");
    ngf::Sprite sprite;
    sprite.getTransform().setPosition(viewCenter);
    sprite.setTexture(*m_saveLoadSheet.getTexture());
    sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2),
                                     static_cast<float>(rect.getHeight() / 2)});
    sprite.setTextureRect(rect);
    sprite.draw(target, {});

    viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
    target.setView(ngf::View(viewRect));

    // heading
    m_headingText.draw(target, {});

    // controls
    for (auto &button : m_buttons) {
      button.draw(target, {});
    }
    target.setView(view);
  }

  void update(const ngf::TimeSpan &elapsed) {
    auto pos = m_pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(
        ngf::Mouse::getPosition(),
        ngf::View(ngf::frect::fromPositionSize(
            {0, 0}, {Screen::Width, Screen::Height})));
    for (auto &button : m_buttons) {
      button.update(elapsed, pos);
    }
  }
};

QuitDialog::QuitDialog()
  : m_pImpl(std::make_unique<Impl>()) {
}

QuitDialog::~QuitDialog() = default;

void QuitDialog::setEngine(Engine *pEngine) { m_pImpl->setEngine(pEngine); }

void QuitDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_pImpl->draw(target, states);
}

void QuitDialog::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->update(elapsed);
}

void QuitDialog::setCallback(Callback callback) {
  m_pImpl->m_callback = std::move(callback);
}

void QuitDialog::updateLanguage() {
  m_pImpl->updateState();
}
}
