#include <filesystem>
#include <string>
#include <engge/UI/SaveLoadDialog.hpp>
#include "_ControlConstants.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Util/Util.hpp>
#include "_Button.hpp"
#include <imgui.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/Graphics/RectangleShape.h>
#include <engge/Graphics/Text.hpp>
#include <engge/Graphics/FntFont.h>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/System/Locator.hpp>

namespace ng {
struct SaveLoadDialog::Impl {
  class BackButton final : public ngf::Drawable {
  private:
    inline static const int BackId = 99904;

  public:
    typedef std::function<void()> Callback;

    void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
      _text.draw(target, states);
    }

  public:
    void setCallback(Callback callback) {
      _callback = std::move(callback);
    }

    void setEngine(Engine *pEngine) {
      _pEngine = pEngine;

      auto &uiFontLarge = _pEngine->getResourceManager().getFntFont("UIFontLarge.fnt");
      _text.setFont(uiFontLarge);
      _text.setWideString(Engine::getText(BackId));
      auto textRect = ng::getGlobalBounds(_text);
      _text.getTransform().setOrigin({textRect.getWidth() / 2.f, textRect.getHeight() / 2.f});
      _text.getTransform().setPosition({Screen::Width / 2.0f, 660.f});
    }

    void update(glm::vec2 pos) {
      auto textRect = ng::getGlobalBounds(_text);
      ngf::Color color;
      if (textRect.contains(pos)) {
        color = _ControlConstants::HoveColor;
        bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
        ImGuiIO &io = ImGui::GetIO();
        if (!io.WantCaptureMouse && _wasMouseDown && !isDown && _callback) {
          _callback();
        }
        _wasMouseDown = isDown;
      } else {
        color = _ControlConstants::NormalColor;
      }
      _text.setColor(color);
    }

  private:
    Engine *_pEngine{nullptr};
    bool _wasMouseDown{false};
    Callback _callback{nullptr};
    ng::Text _text;
  };

  class Slot final : public ngf::Drawable {
  private:
    inline static const int AutosaveId = 99901;

  public:
    void init(const SavegameSlot &slot, const SpriteSheet &spriteSheet, Engine &engine) {
      _index = slot.slot - 1;
      auto x = _index % 3;
      auto y = _index / 3;
      glm::vec2 pos = {168.f + 39.f * 4.f + 78.f * 4.f * x + 4.f * x,
                       92.f + 22.f * 4.f + 44.f * 4.f * y + 4.f * y};

      _transform.setPosition(pos);

      const auto &uiFontSmallBold = engine.getResourceManager().getFntFont("UIFontSmallBold.fnt");

      // prepare the text for the game time
      _gameTimeText.setWideString(slot.getGameTimeString());
      _gameTimeText.setFont(uiFontSmallBold);
      _gameTimeText.setColor(ngf::Colors::White);
      auto gameTimeSize = _gameTimeText.getLocalBounds();
      _gameTimeText.getTransform().setOrigin({static_cast<float>(gameTimeSize.getWidth() / 2), 0});
      _gameTimeText.getTransform().setPosition({pos.x, pos.y - 88.f});

      // prepare the text for the time when the game has been saved
      std::wstring saveTimeText;
      if (slot.slot == 1) {
        saveTimeText = ng::Engine::getText(AutosaveId);
      } else {
        saveTimeText = slot.getSaveTimeString();
      }
      _saveTimeText.setWideString(saveTimeText);
      _saveTimeText.setFont(uiFontSmallBold);
      _saveTimeText.setColor(ngf::Colors::White);
      auto saveTimeSize = _saveTimeText.getLocalBounds();
      _saveTimeText.getTransform().setOrigin({static_cast<float>(saveTimeSize.getWidth() / 2), 0});
      _saveTimeText.getTransform().setPosition({pos.x, pos.y + 48.f});

      // prepare the sprite for the frame
      auto rect = spriteSheet.getRect("saveload_slot_frame");
      _sprite.setTexture(*spriteSheet.getTexture());
      _sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2.f),
                                        static_cast<float>(rect.getHeight() / 2.f)});
      _sprite.getTransform().setScale({4, 4});
      _sprite.setTextureRect(rect);
      _sprite.getTransform().setPosition(pos);

      // try to find the savegame thumbnail
      std::ostringstream s;
      s << "Savegame" << (_index + 1) << ".png";
      auto path = Locator<EngineSettings>::get().getPath();
      path.append(s.str());

      _isEmpty = !std::filesystem::exists(path);
      if (!_isEmpty) {
        // prepare a sprite for the savegame thumbnail
        _texture.load(path);
        _spriteImg.setTexture(_texture, true);
        _spriteImg.getTransform().setOrigin({160.f, 90.f});
        auto size = _texture.getSize();
        _spriteImg.getTransform().setScale({(rect.getWidth() * 4.f) / size.x, (rect.getHeight() * 4.f) / size.y});
        _spriteImg.getTransform().setPosition(pos);
        return;
      }

      // or prepare a sprite for the savegame empty slot
      auto saveslotRect = spriteSheet.getRect("saveload_slot");
      _spriteImg.setTexture(*spriteSheet.getTexture());
      _spriteImg.setTextureRect(saveslotRect);
      _spriteImg.getTransform().setOrigin({static_cast<float>(saveslotRect.getWidth() / 2.f),
                                           static_cast<float>(saveslotRect.getHeight() / 2.f)});
      _spriteImg.getTransform().setScale({4.f, 4.f});
      _spriteImg.getTransform().setPosition(pos);
    }

    bool contains(const glm::vec2 &pos) const {
      auto trsf = _transform;
      trsf.move({-156.f, -88.f});
      return ngf::transform(trsf.getTransform(), _rect).contains(pos);
    }

    inline bool isEmpty() const {
      return _isEmpty;
    }

    void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override {
      _spriteImg.draw(target, states);
      _sprite.draw(target, states);
      if (!_isEmpty) {
        _gameTimeText.draw(target, states);
        _saveTimeText.draw(target, states);
      }
    }

  private:
    int _index{0};
    bool _isEmpty{true};
    ngf::Texture _texture;
    ngf::Sprite _sprite, _spriteImg;
    ng::Text _gameTimeText;
    ng::Text _saveTimeText;
    ngf::Transform _transform;
    ngf::frect _rect = ngf::frect::fromPositionSize({0, 0}, {78 * 4, 44 * 4});
  };

  inline static const int LoadGameId = 99910;
  inline static const int SaveGameId = 99911;

  Engine *_pEngine{nullptr};
  SpriteSheet _saveLoadSheet;
  ng::Text _headingText;
  SaveLoadDialog::Impl::BackButton _backButton;
  Callback _callback{nullptr};
  SlotCallback _slotCallback{nullptr};
  std::array<Slot, 9> _slots;
  bool _wasMouseDown{false};
  bool _saveMode{false};

  void setHeading(bool saveMode) {
    _headingText.setWideString(Engine::getText(saveMode ? SaveGameId : LoadGameId));
    auto textRect = _headingText.getLocalBounds();
    _headingText.getTransform().setOrigin({textRect.getWidth() / 2.f, 0});
    _headingText.getTransform().setPosition({Screen::Width / 2.f, 32.f});
  }

  void updateState() {
    std::vector<SavegameSlot> slots;
    ng::Engine::getSlotSavegames(slots);

    for (int i = 0; i < static_cast<int>(_slots.size()); ++i) {
      _slots[i].init(slots[i], _saveLoadSheet, *_pEngine);
    }

    _wasMouseDown = false;
    setHeading(_saveMode);

    _backButton.setCallback([this]() {
      if (_callback)
        _callback();
    });

    _backButton.setEngine(_pEngine);
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    if (!pEngine)
      return;

    ResourceManager &tm = pEngine->getResourceManager();
    _saveLoadSheet.setTextureManager(&tm);
    _saveLoadSheet.load("SaveLoadSheet");

    auto &headingFont = _pEngine->getResourceManager().getFntFont("HeadingFont.fnt");
    _headingText.setFont(headingFont);
    _headingText.setColor(ngf::Colors::White);
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
    auto rect = _saveLoadSheet.getRect("saveload");
    ngf::Sprite sprite;
    sprite.getTransform().setPosition(viewCenter);
    sprite.setTexture(*_saveLoadSheet.getTexture());
    sprite.getTransform().setOrigin({static_cast<float>(rect.getWidth() / 2.f),
                                     static_cast<float>(rect.getHeight() / 2.f)});
    sprite.setTextureRect(rect);
    sprite.draw(target, {});

    viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
    target.setView(ngf::View(viewRect));

    // heading
    _headingText.draw(target);

    // slots
    for (auto &slot : _slots) {
      slot.draw(target, {});
    }

    // back button
    _backButton.draw(target, {});

    target.setView(view);
  }

  void update(const ngf::TimeSpan &) {
    auto pos = _pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
                                                                               ngf::View(ngf::frect::fromPositionSize({0,
                                                                                                                       0},
                                                                                                                      {Screen::Width,
                                                                                                                       Screen::Height})));
    _backButton.update(pos);

    bool isDown = ngf::Mouse::isButtonPressed(ngf::Mouse::Button::Left);
    const ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && _wasMouseDown && !isDown) {
      int i = 0;
      for (const auto &slot : _slots) {
        if (slot.contains(pos)) {
          if ((_saveMode || !slot.isEmpty()) && _slotCallback) {
            _slotCallback(i + 1);
          }
          return;
        }
        i++;
      }
    }
    _wasMouseDown = isDown;
  }
};

SaveLoadDialog::SaveLoadDialog()
    : _pImpl(std::make_unique<Impl>()) {
}

SaveLoadDialog::~SaveLoadDialog() = default;

void SaveLoadDialog::setEngine(Engine *pEngine) { _pImpl->setEngine(pEngine); }

void SaveLoadDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  _pImpl->draw(target, states);
}

void SaveLoadDialog::update(const ngf::TimeSpan &elapsed) {
  _pImpl->update(elapsed);
}

void SaveLoadDialog::setCallback(Callback callback) {
  _pImpl->_callback = std::move(callback);
}

void SaveLoadDialog::setSlotCallback(SlotCallback callback) {
  _pImpl->_slotCallback = std::move(callback);
}

void SaveLoadDialog::updateLanguage() {
  _pImpl->updateState();
}

void SaveLoadDialog::setSaveMode(bool saveMode) {
  _pImpl->_saveMode = saveMode;
  _pImpl->setHeading(saveMode);
}

bool SaveLoadDialog::getSaveMode() const { return _pImpl->_saveMode; }
}