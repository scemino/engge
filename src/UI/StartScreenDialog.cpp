#include "Button.hpp"
#include <engge/EnggeApplication.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/UI/OptionsDialog.hpp>
#include <engge/UI/SaveLoadDialog.hpp>
#include <engge/UI/StartScreenDialog.hpp>
#include <utility>
#include <ngf/System/Mouse.h>
#include <engge/UI/QuitDialog.hpp>

namespace ng {
struct StartScreenDialog::Impl {
  enum class State { None, Main, Options, Help, Quit };

  struct Ids {
    inline static const int LoadGame = 99910;
    inline static const int NewGame = 99912;
    inline static const int Options = 99913;
    inline static const int Quit = 99915;
    inline static const int Help = 99961;
  };

  static constexpr float yPosStart = 155.f;
  static constexpr float yStep = 80.f;

  Engine *m_pEngine{nullptr};

  std::vector<Button> m_buttons;
  QuitDialog m_quit;
  OptionsDialog m_options;
  SaveLoadDialog m_saveload;
  State m_state{State::None};
  State m_nextState{State::None};
  Callback m_newGameCallback;

  inline static constexpr float getSlotPos(int slot) {
    return yPosStart + yStep * static_cast<float>(slot - 1);
  }

  void setState(State state) {
    m_nextState = state;
  }

  void setEngine(Engine *pEngine) {
    m_pEngine = pEngine;
    if (!pEngine)
      return;

    m_saveload.setEngine(pEngine);
    m_saveload.setCallback([this]() {
      _showSaveLoad = false;
    });

    m_options.setEngine(pEngine);
    m_options.setCallback([this]() {
      setState(State::Main);
    });

    m_quit.setEngine(pEngine);
    m_quit.setCallback([this](bool result) {
      if (result)
        m_pEngine->quit();
      setState(State::Main);
    });

    setState(State::Main);
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) {
    switch (m_state) {
    case State::Main: {
      const auto view = target.getView();
      auto viewRect = ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height});
      target.setView(ngf::View(viewRect));

      // controls
      for (auto &button : m_buttons) {
        button.draw(target, {});
      }

      target.setView(view);

      if (_showSaveLoad) {
        m_saveload.draw(target, states);
      }
      break;
    }

    case State::Options:
    case State::Help:m_options.draw(target, states);
      break;

    case State::Quit:m_quit.draw(target, states);
      break;
    case State::None:break;
    }
  }

  void onStateChanged() {
    m_buttons.clear();
    switch (m_state) {
    case State::Main:m_buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() { _showSaveLoad = true; });
      m_buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() {
        m_saveload.updateLanguage();
        m_saveload.setSaveMode(false);
        _showSaveLoad = true;
      });
      m_buttons.emplace_back(Ids::NewGame, getSlotPos(2), [this]() {
        if (m_newGameCallback)
          m_newGameCallback();
      });
      m_buttons.emplace_back(Ids::Options, getSlotPos(3), [this]() { setState(State::Options); });
      m_buttons.emplace_back(Ids::Help, getSlotPos(4), [this]() { setState(State::Help); });
      m_buttons.emplace_back(Ids::Quit, getSlotPos(5), [this]() { setState(State::Quit); });
      break;
    case State::Help:m_options.showHelp();
      break;
    case State::Options:break;
    case State::Quit:break;
    default:setState(State::Main);
      break;
    }

    for (auto &button : m_buttons) {
      button.setEngine(m_pEngine);
    }
  }

  void update(const ngf::TimeSpan &elapsed) {
    if (m_nextState != m_state) {
      m_state = m_nextState;
      onStateChanged();
    }

    switch (m_state) {
    case State::Quit:m_quit.update(elapsed);
      break;

    case State::Options:
    case State::Help:m_options.update(elapsed);
      break;

    default:
      if (_showSaveLoad) {
        m_saveload.update(elapsed);
        return;
      }

      auto pos = m_pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
                                                                                  ngf::View(ngf::frect::fromPositionSize(
                                                                                      {0,
                                                                                       0},
                                                                                      {Screen::Width,
                                                                                       Screen::Height})));
      for (auto &button : m_buttons) {
        button.update(elapsed, pos);
      }
      break;
    }
  }
  bool _showSaveLoad{false};
};

StartScreenDialog::StartScreenDialog()
    : m_pImpl(std::make_unique<Impl>()) {
}

StartScreenDialog::~StartScreenDialog() = default;

void StartScreenDialog::setEngine(Engine *pEngine) { m_pImpl->setEngine(pEngine); }

void StartScreenDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_pImpl->draw(target, states);
}

void StartScreenDialog::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->update(elapsed);
}

void StartScreenDialog::setNewGameCallback(Callback callback) {
  m_pImpl->m_newGameCallback = std::move(callback);
}

void StartScreenDialog::setSlotCallback(SaveLoadDialog::SlotCallback callback) {
  m_pImpl->m_saveload.setSlotCallback(std::move(callback));
}
}
