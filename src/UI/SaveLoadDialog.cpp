#include <filesystem>
#include <string>
#include <engge/EnggeApplication.hpp>
#include <engge/UI/SaveLoadDialog.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <ngf/Graphics/Sprite.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Text.h>
#include <ngf/Graphics/FntFont.h>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/System/Locator.hpp>
#include <engge/Engine/TextDatabase.hpp>
#include <ngf/System/Mouse.h>
#include "Button.hpp"
#include "ControlConstants.hpp"
#include "Util/Util.hpp"

namespace ng {
struct SaveLoadDialog::Impl {
  class BackButton final : public Control {
  private:
    inline static const int BackId = 99904;

  public:
    typedef std::function<void()> Callback;

    void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
      m_text.draw(target, states);
    }

  public:
    void setCallback(Callback callback) {
      m_callback = std::move(callback);
    }

    void onEngineSet() final {
      m_text.setFont(m_pEngine->getResourceManager().getFntFont("UIFontLarge.fnt"));
      m_text.setWideString(Engine::getText(BackId));
      auto textRect = ng::getGlobalBounds(m_text);
      m_text.getTransform().setOrigin({textRect.getWidth() / 2.f, textRect.getHeight() / 2.f});
      m_text.getTransform().setPosition({Screen::Width / 2.0f, 660.f});
    }

    bool contains(glm::vec2 pos) const final {
      auto textRect = ng::getGlobalBounds(m_text);
      return textRect.contains((glm::vec2) pos);
    }

    void onClick() final {
      if (m_callback) {
        m_callback();
      }
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

    void update(const ngf::TimeSpan &elapsed, glm::vec2 pos) final {
      Control::update(elapsed, pos);
      m_text.getTransform().setPosition(m_shakeOffset + glm::vec2{Screen::Width / 2.0f, 660.f});
    }

  private:
    Callback m_callback{nullptr};
    ng::Text m_text;
  };

  class Slot final : public Control {
  private:
    inline static const int AutosaveId = 99901;

  public:
    void init(const SavegameSlot &slot, const SpriteSheet &spriteSheet, Engine &engine) {
      setEngine(&engine);
      m_index = slot.slot - 1;
      auto x = m_index % 3;
      auto y = m_index / 3;
      glm::vec2 pos = {168.f + 39.f * 4.f + 78.f * 4.f * x + 4.f * x,
                       92.f + 22.f * 4.f + 44.f * 4.f * y + 4.f * y};

      m_transform.setPosition(pos);

      const auto &uiFontSmallBold = engine.getResourceManager().getFntFont("UIFontSmallBold.fnt");

      // prepare the text for the game time
      m_gameTimeText.setWideString(slot.getGameTimeString());
      m_gameTimeText.setFont(uiFontSmallBold);
      m_gameTimeText.setColor(ngf::Colors::White);
      auto gameTimeSize = m_gameTimeText.getLocalBounds();
      m_gameTimeText.getTransform().setOrigin({static_cast<float>(gameTimeSize.getWidth() / 2), 0});
      m_gameTimeText.getTransform().setPosition({pos.x, pos.y - 88.f});

      // prepare the text for the time when the game has been saved
      std::wstring saveTimeText;
      if (slot.slot == 1) {
        saveTimeText = ng::Engine::getText(AutosaveId);
      } else {
        saveTimeText = slot.getSaveTimeString();
      }
      if (slot.easyMode) {
        saveTimeText.append(1, L' ');
        saveTimeText.append(Locator<TextDatabase>::get().getText(99955));
      }
      m_saveTimeText.setWideString(saveTimeText);
      m_saveTimeText.setFont(uiFontSmallBold);
      m_saveTimeText.setColor(ngf::Colors::White);
      auto saveTimeSize = m_saveTimeText.getLocalBounds();
      m_saveTimeText.getTransform().setOrigin({static_cast<float>(saveTimeSize.getWidth() / 2), 0});
      m_saveTimeText.getTransform().setPosition({pos.x, pos.y + 48.f});

      // prepare the sprite for the frame
      auto rect = spriteSheet.getRect("saveload_slot_frame");
      m_sprite.setTexture(*spriteSheet.getTexture());
      m_sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2.f),
                                         static_cast<float>(rect.getHeight() / 2.f)});
      m_sprite.getTransform().setScale({4, 4});
      m_sprite.setTextureRect(rect);
      m_sprite.getTransform().setPosition(pos);

      // try to find the savegame thumbnail
      std::ostringstream s;
      s << "Savegame" << (m_index + 1) << ".png";
      auto path = Locator<EngineSettings>::get().getPath();
      path.append(s.str());

      m_isEmpty = !std::filesystem::exists(path);
      if (!m_isEmpty) {
        // prepare a sprite for the savegame thumbnail
        m_texture.load(path);
        m_spriteImg.setTexture(m_texture, true);
        m_spriteImg.getTransform().setOrigin({160.f, 90.f});
        auto size = m_texture.getSize();
        m_spriteImg.getTransform().setScale({(rect.getWidth() * 4.f) / size.x, (rect.getHeight() * 4.f) / size.y});
        m_spriteImg.getTransform().setPosition(pos);
        return;
      }

      // or prepare a sprite for the savegame empty slot
      auto saveslotRect = spriteSheet.getRect("saveload_slot");
      m_spriteImg.setTexture(*spriteSheet.getTexture());
      m_spriteImg.setTextureRect(saveslotRect);
      m_spriteImg.getTransform().setOrigin({static_cast<float>(saveslotRect.getWidth() / 2.f),
                                            static_cast<float>(saveslotRect.getHeight() / 2.f)});
      m_spriteImg.getTransform().setScale({4.f, 4.f});
      m_spriteImg.getTransform().setPosition(pos);
    }

    bool contains(glm::vec2 pos) const final {
      auto trsf = m_transform;
      trsf.move({-156.f, -88.f});
      return ngf::transform(trsf.getTransform(), m_rect).contains(pos);
    }

    inline bool isEmpty() const {
      return m_isEmpty;
    }

    void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final {
      m_spriteImg.draw(target, states);
      m_sprite.draw(target, states);
      if (!m_isEmpty) {
        m_gameTimeText.draw(target, states);
        m_saveTimeText.draw(target, states);
      }
    }

  private:
    int m_index{0};
    bool m_isEmpty{true};
    ngf::Texture m_texture;
    ngf::Sprite m_sprite, m_spriteImg;
    ng::Text m_gameTimeText;
    ng::Text m_saveTimeText;
    ngf::Transform m_transform;
    ngf::frect m_rect = ngf::frect::fromPositionSize({0, 0}, {78 * 4, 44 * 4});
  };

  inline static const int LoadGameId = 99910;
  inline static const int SaveGameId = 99911;

  Engine *m_pEngine{nullptr};
  SpriteSheet m_saveLoadSheet;
  ng::Text m_headingText;
  SaveLoadDialog::Impl::BackButton m_backButton;
  Callback m_callback{nullptr};
  SlotCallback m_slotCallback{nullptr};
  std::array<Slot, 9> m_slots;
  bool m_wasMouseDown{false};
  bool m_saveMode{false};

  void setHeading(bool saveMode) {
    m_headingText.setWideString(Engine::getText(saveMode ? SaveGameId : LoadGameId));
    auto textRect = m_headingText.getLocalBounds();
    m_headingText.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
    m_headingText.getTransform().setPosition({Screen::Width / 2.f, 32.f});
  }

  void updateState() {
    std::vector<SavegameSlot> slots;
    ng::Engine::getSlotSavegames(slots);

    for (int i = 0; i < static_cast<int>(m_slots.size()); ++i) {
      m_slots[i].init(slots[i], m_saveLoadSheet, *m_pEngine);
    }

    m_wasMouseDown = false;
    setHeading(m_saveMode);

    m_backButton.setCallback([this]() {
      if (m_callback)
        m_callback();
    });

    m_backButton.setEngine(m_pEngine);
  }

  void setEngine(Engine *pEngine) {
    m_pEngine = pEngine;
    if (!pEngine)
      return;

    ResourceManager &tm = pEngine->getResourceManager();
    m_saveLoadSheet.setTextureManager(&tm);
    m_saveLoadSheet.load("SaveLoadSheet");

    auto &headingFont = m_pEngine->getResourceManager().getFntFont("HeadingFont.fnt");
    m_headingText.setFont(headingFont);
    m_headingText.setColor(ngf::Colors::White);
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
    auto rect = m_saveLoadSheet.getRect("saveload");
    ngf::Sprite sprite;
    sprite.getTransform().setPosition(viewCenter);
    sprite.setTexture(*m_saveLoadSheet.getTexture());
    sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2.f),
                                     static_cast<float>(rect.getHeight() / 2.f)});
    sprite.setTextureRect(rect);
    sprite.draw(target, {});

    viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
    target.setView(ngf::View(viewRect));

    // heading
    m_headingText.draw(target, {});

    // slots
    for (auto &slot : m_slots) {
      slot.draw(target, {});
    }

    // back button
    m_backButton.draw(target, {});

    target.setView(view);
  }

  void update(const ngf::TimeSpan &elapsed) {
    auto pos = m_pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(
        ngf::Mouse::getPosition(),
        ngf::View(ngf::frect::fromPositionSize(
            {0, 0}, {Screen::Width, Screen::Height})));
    m_backButton.update(elapsed, pos);
    for (auto &slot : m_slots) {
      slot.update(elapsed, pos);
    }

    bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
    const ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && m_wasMouseDown && !isDown) {
      int i = 0;
      for (const auto &slot : m_slots) {
        if (slot.contains(pos) && m_slotCallback) {
          if (m_saveMode) {
            if (i != 0) {
              m_slotCallback(i + 1);
            }
          } else if (!slot.isEmpty()) {
            m_slotCallback(i + 1);
          }
          return;
        }
        i++;
      }
    }
    m_wasMouseDown = isDown;
  }
};

SaveLoadDialog::SaveLoadDialog()
    : m_pImpl(std::make_unique<Impl>()) {
}

SaveLoadDialog::~SaveLoadDialog() = default;

void SaveLoadDialog::setEngine(Engine *pEngine) { m_pImpl->setEngine(pEngine); }

void SaveLoadDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_pImpl->draw(target, states);
}

void SaveLoadDialog::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->update(elapsed);
}

void SaveLoadDialog::setCallback(Callback callback) {
  m_pImpl->m_callback = std::move(callback);
}

void SaveLoadDialog::setSlotCallback(SlotCallback callback) {
  m_pImpl->m_slotCallback = std::move(callback);
}

void SaveLoadDialog::updateLanguage() {
  m_pImpl->updateState();
}

void SaveLoadDialog::setSaveMode(bool saveMode) {
  m_pImpl->m_saveMode = saveMode;
  m_pImpl->setHeading(saveMode);
}

bool SaveLoadDialog::getSaveMode() const { return m_pImpl->m_saveMode; }
}