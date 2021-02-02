#include "Button.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/UI/OptionsDialog.hpp"
#include "engge/UI/SaveLoadDialog.hpp"
#include "engge/UI/StartScreenDialog.hpp"
#include <utility>
#include <ngf/System/Mouse.h>
#include "engge/UI/QuitDialog.hpp"

namespace ng {
struct StartScreenDialog::Impl {
  enum class State { Main, Options, Help, Quit };

  struct Ids {
    inline static const int LoadGame = 99910;
    inline static const int NewGame = 99912;
    inline static const int Options = 99913;
    inline static const int Quit = 99915;
    inline static const int Help = 99961;
  };

  static constexpr float yPosStart = 84.f;
  static constexpr float yPosLarge = 58.f;
  static constexpr float yPosSmall = 54.f;

  Engine *_pEngine{nullptr};

  std::vector<Button> _buttons;
  QuitDialog _quit;
  OptionsDialog _options;
  SaveLoadDialog _saveload;
  State _state{State::Main};
  Callback _newGameCallback;

  inline static float getSlotPos(int slot) {
    return yPosStart + yPosLarge + yPosSmall * slot;
  }

  void updateState(State state) {
    _state = state;
    _buttons.clear();
    switch (state) {
    case State::Main:_buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() { _showSaveLoad = true; });
      _buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() {
        _saveload.updateLanguage();
        _saveload.setSaveMode(false);
        _showSaveLoad = true;
      });
      _buttons.emplace_back(Ids::NewGame, getSlotPos(2), [this]() {
        if (_newGameCallback)
          _newGameCallback();
      });
      _buttons.emplace_back(Ids::Options, getSlotPos(3), [this]() { updateState(State::Options); });
      _buttons.emplace_back(Ids::Help, getSlotPos(4), [this]() { updateState(State::Help); });
      _buttons.emplace_back(Ids::Quit, getSlotPos(5), [this]() { updateState(State::Quit); });
      break;
    case State::Help:_options.showHelp();
      break;
    case State::Options:break;
    case State::Quit:break;
    default:updateState(State::Main);
      break;
    }

    for (auto &button : _buttons) {
      button.setEngine(_pEngine);
    }
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    if (!pEngine)
      return;

    _saveload.setEngine(pEngine);
    _saveload.setCallback([this]() {
      _showSaveLoad = false;
    });

    _options.setEngine(pEngine);
    _options.setCallback([this]() {
      updateState(State::Main);
    });

    _quit.setEngine(pEngine);
    _quit.setCallback([this](bool result) {
      if (result)
        _pEngine->quit();
      updateState(State::Main);
    });

    updateState(State::Main);
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) {
    switch (_state) {
    case State::Main: {
      const auto view = target.getView();
      auto viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
      target.setView(ngf::View(viewRect));

      // controls
      for (auto &button : _buttons) {
        button.draw(target, {});
      }

      target.setView(view);

      if (_showSaveLoad) {
        _saveload.draw(target, states);
      }
      break;
    }

    case State::Options:
    case State::Help:_options.draw(target, states);
      break;

    case State::Quit:_quit.draw(target, states);
      break;
    }
  }

  void update(const ngf::TimeSpan &elapsed) {
    switch (_state) {
    case State::Quit:_quit.update(elapsed);
      break;

    case State::Options:
    case State::Help:_options.update(elapsed);
      break;

    default:
      if (_showSaveLoad) {
        _saveload.update(elapsed);
        return;
      }

      auto pos = _pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
                                                                                 ngf::View(ngf::frect::fromPositionSize(
                                                                                     {0,
                                                                                      0},
                                                                                     {Screen::Width,
                                                                                      Screen::Height})));
      for (auto &button : _buttons) {
        button.update(pos);
      }
      break;
    }
  }
  bool _showSaveLoad{false};
};

StartScreenDialog::StartScreenDialog()
    : _pImpl(std::make_unique<Impl>()) {
}

StartScreenDialog::~StartScreenDialog() = default;

void StartScreenDialog::setEngine(Engine *pEngine) { _pImpl->setEngine(pEngine); }

void StartScreenDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  _pImpl->draw(target, states);
}

void StartScreenDialog::update(const ngf::TimeSpan &elapsed) {
  _pImpl->update(elapsed);
}

void StartScreenDialog::setNewGameCallback(Callback callback) {
  _pImpl->_newGameCallback = std::move(callback);
}

void StartScreenDialog::setSlotCallback(SaveLoadDialog::SlotCallback callback) {
  _pImpl->_saveload.setSlotCallback(std::move(callback));
}
}
