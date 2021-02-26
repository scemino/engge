#include "HelpDialog.hpp"
#include "Button.hpp"
#include "Control.hpp"
#include "ControlConstants.hpp"
#include <engge/EnggeApplication.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <ngf/Graphics/FntFont.h>
#include <ngf/System/Mouse.h>
#include <utility>
#include "Util/Util.hpp"

namespace ng {

class HelpButton final : public Control {
public:
  using Callback = std::function<void()>;
  enum class Size { Large, Medium };

  HelpButton(int id, glm::vec2 pos = {0, 0}, Callback callback = nullptr, bool enabled = true, Size size = Size::Large)
      : Control(enabled), m_id(id), m_pos(pos), m_callback(callback), m_size(size) {
    m_text.setColor(enabled ? ControlConstants::NormalColor : ControlConstants::DisabledColor);
  }

  ~HelpButton() final = default;

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
    if (!m_isVisible)
      return;
    m_text.draw(target, states);
  }

  void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final {
    Control::update(elapsed, pos);
    m_text.getTransform().setPosition(m_shakeOffset + m_pos);
  }

  void setCallback(Callback callback) { m_callback = callback; }

  void setVisible(bool visible) { m_isVisible = visible; }
  bool isVisible() const { return m_isVisible; }

  void setPosition(const glm::vec2 &pos) { m_pos = pos; }
  glm::vec2 getPosition() const { return m_pos; }

  void setAnchor(ngf::Anchor anchor) {
    m_text.setAnchor(anchor);
  }

private:
  bool contains(glm::vec2 pos) const final {
    if (!m_isVisible)
      return false;
    auto p = ngf::transform(glm::inverse(m_text.getTransform().getTransform()), pos);
    return m_text.getLocalBounds().contains(p);
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

  void onEngineSet() final {
    const auto &uiFontLargeOrMedium =
        m_pEngine->getResourceManager().getFntFont(m_size == Size::Large ? "UIFontLarge.fnt" : "UIFontMedium.fnt");
    m_text.setFont(uiFontLargeOrMedium);
    m_text.setWideString(ng::Engine::getText(m_id));
    m_text.getTransform().setPosition(m_pos);
  }

  void onClick() final {
    if (m_callback) {
      m_callback();
    }
  }

private:
  int m_id{0};
  glm::vec2 m_pos{0, 0};
  Callback m_callback{nullptr};
  ng::Text m_text;
  Size m_size{Size::Large};
  bool m_isVisible{true};
};

struct HelpDialog::Impl {
  struct Ids {
    static constexpr int Back = 99904;
    static constexpr int Next = 99962;
    static constexpr int Prev = 99963;
  };

  Engine *m_pEngine{nullptr};
  HelpButton m_back;
  HelpButton m_prev;
  HelpButton m_next;
  std::vector<int> m_pages;
  ngf::Sprite m_backgroundSprite;
  ngf::Sprite m_helpPageSprite;
  int m_pageIndex{0};

  Impl()
      : m_back{Ids::Back, {0, 0}, nullptr, true, HelpButton::Size::Medium},
        m_prev{Ids::Prev, {0, 0}, nullptr, true, HelpButton::Size::Medium},
        m_next{Ids::Next, {300, 0}, nullptr, true, HelpButton::Size::Medium} {
    m_prev.setCallback([this]() { prevPage(); });
    m_next.setCallback([this]() { nextPage(); });
  }

  void nextPage() {
    displayPage(++m_pageIndex);
  }

  void prevPage() {
    displayPage(--m_pageIndex);
  }

  void init(Engine *pEngine, Callback exitCallback, std::initializer_list<int> pages) {
    m_pEngine = pEngine;
    if (!pEngine)
      return;

    m_pages = pages;
    m_back.setEngine(pEngine);
    m_back.setCallback(exitCallback);
    m_back.setPosition({50.f, 16.f});
    m_prev.setEngine(pEngine);
    m_next.setEngine(pEngine);

    m_backgroundSprite.setTexture(*m_pEngine->getResourceManager().getTexture("HelpScreen_bg"));
    m_backgroundSprite.getTransform().setPosition({Screen::HalfWidth, Screen::HalfHeight});
    m_backgroundSprite.setAnchor(ngf::Anchor::Center);

    m_helpPageSprite.getTransform().setPosition({Screen::HalfWidth, Screen::HalfHeight});

    displayPage(0);
  }

  void displayPage(int index) {
    m_pageIndex = index;
    m_prev.setVisible(m_pageIndex > 0);
    m_prev.setAnchor(ngf::Anchor::BottomLeft);
    m_prev.setPosition({30.f, Screen::Height - 30.f});

    m_next.setVisible(m_pageIndex < static_cast<int>(m_pages.size() - 1));
    m_next.setAnchor(ngf::Anchor::BottomRight);
    m_next.setPosition({Screen::Width - 32.f, Screen::Height - 32.f});

    char background[17];
    sprintf(background, "HelpScreen_%02d_en", m_pages[index]);
    std::string backgroundWithLang = background;
    checkLanguage(backgroundWithLang);
    m_helpPageSprite.setTexture(*m_pEngine->getResourceManager().getTexture(backgroundWithLang));
    m_helpPageSprite.setAnchor(ngf::Anchor::Center);
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) {
    const auto view = target.getView();
    auto viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
    target.setView(ngf::View(viewRect));

    m_backgroundSprite.draw(target, states);
    m_helpPageSprite.draw(target, states);
    m_back.draw(target, states);
    m_prev.draw(target, states);
    m_next.draw(target, states);

    target.setView(view);
  }

  void update(const ngf::TimeSpan &elapsed) {
    auto pos = m_pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(
        ngf::Mouse::getPosition(),
        ngf::View(ngf::frect::fromPositionSize(
            {0, 0}, {Screen::Width, Screen::Height})));
    m_back.update(elapsed, pos);
    m_prev.update(elapsed, pos);
    m_next.update(elapsed, pos);
  }
};

HelpDialog::HelpDialog()
    : m_pImpl(std::make_unique<Impl>()) {
}

HelpDialog::~HelpDialog() = default;

void HelpDialog::init(Engine *pEngine, Callback exitCallback, std::initializer_list<int> pages) {
  m_pImpl->init(pEngine, exitCallback, pages);
}

void HelpDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_pImpl->draw(target, states);
}

void HelpDialog::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->update(elapsed);
}
}
