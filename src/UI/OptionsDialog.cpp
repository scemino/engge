#include "Button.hpp"
#include "Checkbox.hpp"
#include "Slider.hpp"
#include "SwitchButton.hpp"
#include "engge/Audio/SoundManager.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/SpriteSheet.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "engge/System/Logger.hpp"
#include "engge/UI/OptionsDialog.hpp"
#include "engge/UI/SaveLoadDialog.hpp"
#include "engge/UI/QuitDialog.hpp"
#include <utility>
#include <ngf/Graphics/RectangleShape.h>

namespace ng {
struct OptionsDialog::Impl {
  enum class State { Main, Sound, Video, Controls, TextAndSpeech, Help };

  struct Ids {
    inline static const int EnglishText = 98001;
    inline static const int FrenchText = 98003;
    inline static const int ItalianText = 98005;
    inline static const int GermanText = 98007;
    inline static const int SpanishText = 98009;
    inline static const int Back = 99904;
    inline static const int LoadGame = 99910;
    inline static const int SaveGame = 99911;
    inline static const int Options = 99913;
    inline static const int Credits = 99914;
    inline static const int Quit = 99915;
    inline static const int Sound = 99916;
    inline static const int Video = 99917;
    inline static const int Controls = 99918;
    inline static const int TextAndSpeech = 99919;
    inline static const int Fullscreen = 99927;
    inline static const int SafeArea = 99929;
    inline static const int RetroFonts = 99933;
    inline static const int RetroVerbs = 99934;
    inline static const int ClassicSentence = 99935;
    inline static const int SoundVolume = 99937;
    inline static const int MusicVolume = 99938;
    inline static const int VoiceVolume = 99939;
    inline static const int Controller = 99940;
    inline static const int TextSpeed = 99941;
    inline static const int DisplayText = 99942;
    inline static const int HearVoice = 99943;
    inline static const int ScrollSyncCursor = 99960;
    inline static const int Help = 99961;
    inline static const int InvertVerbColors = 99964;
    inline static const int ToiletPaperOver = 99965;
    inline static const int Introduction = 99966;
    inline static const int MouseTips = 99967;
    inline static const int ControllerTips = 99968;
    inline static const int ControllerMap = 99969;
    inline static const int KeyboardMap = 99970;
    inline static const int AnnoyingInJokes = 99971;
  };

  inline static const std::array<std::string, 5> LanguageValues = {"en", "fr", "it", "de", "es"};
  static constexpr float yPosStart = 84.f;
  static constexpr float yPosLarge = 58.f;
  static constexpr float yPosSmall = 54.f;

  Engine *_pEngine{nullptr};
  SpriteSheet _saveLoadSheet;

  ng::Text _headingText;
  std::vector<Button> _buttons;
  std::vector<SwitchButton> _switchButtons;
  std::vector<Checkbox> _checkboxes;
  std::vector<Slider> _sliders;
  bool _showQuit{false};
  bool _showSaveLoad{false};
  QuitDialog _quit;
  SaveLoadDialog _saveload;
  Callback _callback{nullptr};
  bool _isDirty{false};
  State _state{State::Main};
  bool _saveEnabled{false};

  inline static float getSlotPos(int slot) {
    return yPosStart + yPosLarge + yPosSmall * static_cast<float>(slot);
  }

  void setHeading(int id) {
    _headingText.setWideString(Engine::getText(id));
    auto textRect = _headingText.getLocalBounds();
    _headingText.getTransform().setPosition({(Screen::Width - textRect.getWidth()) / 2.f, yPosStart - textRect.getHeight() / 2});
  }

  template<typename T>
  void setUserPreference(const std::string &name, T value) {
    Locator<Preferences>::get().setUserPreference(name, value);
  }

  template<typename T>
  T getUserPreference(const std::string &name, T value) const {
    return Locator<Preferences>::get().getUserPreference(name, value);
  }

  static int getLanguageUserPreference() {
    auto lang =
        Locator<Preferences>::get().getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
    auto it = std::find(LanguageValues.begin(), LanguageValues.end(), lang);
    return static_cast<int>(std::distance(LanguageValues.begin(), it));
  }

  void updateState(State state) {
    _state = state;
    if (_isDirty) {
      Locator<Preferences>::get().save();
      _quit.updateLanguage();
      _isDirty = false;
    }
    _sliders.clear();
    _buttons.clear();
    _switchButtons.clear();
    _checkboxes.clear();
    switch (state) {
    case State::Main:setHeading(Ids::Options);
      _buttons.emplace_back(Ids::SaveGame, getSlotPos(0), [this]() {
        _saveload.updateLanguage();
        _saveload.setSaveMode(true);
        _showSaveLoad = true;
      }, _saveEnabled);
      _buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() {
        _saveload.updateLanguage();
        _saveload.setSaveMode(false);
        _showSaveLoad = true;
      });
      _buttons.emplace_back(Ids::Sound, getSlotPos(2), [this]() { updateState(State::Sound); });
      _buttons.emplace_back(Ids::Video, getSlotPos(3), [this]() { updateState(State::Video); });
      _buttons.emplace_back(Ids::Controls, getSlotPos(4), [this]() { updateState(State::Controls); });
      _buttons.emplace_back(Ids::TextAndSpeech, getSlotPos(5), [this]() { updateState(State::TextAndSpeech); });
      _buttons.emplace_back(Ids::Help, getSlotPos(6), [this]() { updateState(State::Help); });
      _buttons.emplace_back(Ids::Quit, getSlotPos(7), [this]() { _showQuit = true; }, true);
      _buttons.emplace_back(Ids::Back, getSlotPos(9), [this]() {
        if (_callback)
          _callback();
      }, true, Button::Size::Medium);
      break;
    case State::Sound:setHeading(Ids::Sound);
      _sliders.emplace_back(Ids::SoundVolume,
                            getSlotPos(2),
                            true,
                            Locator<SoundManager>::get().getSoundVolume(),
                            [this](auto value) {
                              _isDirty = true;
                              Locator<SoundManager>::get().setSoundVolume(value);
                            });
      _sliders.emplace_back(Ids::MusicVolume,
                            getSlotPos(3),
                            true,
                            Locator<SoundManager>::get().getMusicVolume(),
                            [this](auto value) {
                              _isDirty = true;
                              Locator<SoundManager>::get().setMusicVolume(value);
                            });
      _sliders.emplace_back(Ids::VoiceVolume,
                            getSlotPos(4),
                            true,
                            Locator<SoundManager>::get().getTalkVolume(),
                            [this](auto value) {
                              _isDirty = true;
                              Locator<SoundManager>::get().setTalkVolume(value);
                            });
      _buttons.emplace_back(Ids::Back,
                            getSlotPos(9),
                            [this]() { updateState(State::Main); },
                            true,
                            Button::Size::Medium);
      break;
    case State::Video:setHeading(Ids::Video);
      _checkboxes.emplace_back(Ids::Fullscreen, getSlotPos(1), true,
                               getUserPreference(PreferenceNames::Fullscreen, PreferenceDefaultValues::Fullscreen),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::Fullscreen, value);
                               });
      _sliders.emplace_back(Ids::SafeArea, getSlotPos(2), false,
                            getUserPreference(PreferenceNames::SafeArea, PreferenceDefaultValues::SafeArea),
                            [this](auto value) {
                              _isDirty = true;
                              setUserPreference(PreferenceNames::SafeArea, value);
                            });
      _checkboxes.emplace_back(Ids::ToiletPaperOver, getSlotPos(4), true,
                               getUserPreference(PreferenceNames::ToiletPaperOver,
                                                 PreferenceDefaultValues::ToiletPaperOver),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::ToiletPaperOver, value);
                                 ScriptEngine::call("setSettingVar", "toilet_paper_over", value ? 1 : 0);
                               });
      _checkboxes.emplace_back(Ids::AnnoyingInJokes, getSlotPos(5), true,
                               getUserPreference(PreferenceNames::AnnoyingInJokes,
                                                 PreferenceDefaultValues::AnnoyingInJokes),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::AnnoyingInJokes, value);
                                 ScriptEngine::call("setSettingVar", "annoying_injokes", value ? 1 : 0);
                               });
      _buttons.emplace_back(Ids::Back,
                            getSlotPos(9),
                            [this]() { updateState(State::Main); },
                            true,
                            Button::Size::Medium);
      break;
    case State::Controls:setHeading(Ids::Controls);
      _checkboxes.emplace_back(Ids::Controller, getSlotPos(1), false,
                               getUserPreference(PreferenceNames::Controller, PreferenceDefaultValues::Controller),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::Controller, value);
                               });
      _checkboxes.emplace_back(Ids::ScrollSyncCursor, getSlotPos(2), false,
                               getUserPreference(PreferenceNames::ScrollSyncCursor,
                                                 PreferenceDefaultValues::ScrollSyncCursor),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::ScrollSyncCursor, value);
                               });
      _checkboxes.emplace_back(Ids::InvertVerbColors, getSlotPos(4), true,
                               getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                 PreferenceDefaultValues::InvertVerbHighlight),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::InvertVerbHighlight, value);
                               });
      _checkboxes.emplace_back(Ids::RetroFonts, getSlotPos(5), true,
                               getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::RetroFonts, value);
                               });
      _checkboxes.emplace_back(Ids::RetroVerbs, getSlotPos(6), true,
                               getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::RetroVerbs, value);
                                 if (value) {
                                   _checkboxes[3].setChecked(true);
                                   _checkboxes[5].setChecked(true);
                                 }
                               });
      _checkboxes.emplace_back(Ids::ClassicSentence, getSlotPos(7), true,
                               getUserPreference(PreferenceNames::ClassicSentence,
                                                 PreferenceDefaultValues::ClassicSentence),
                               [this](auto value) {
                                 _isDirty = true;
                                 setUserPreference(PreferenceNames::ClassicSentence, value);
                               });
      _buttons.emplace_back(Ids::Back,
                            getSlotPos(9),
                            [this]() { updateState(State::Main); },
                            true,
                            Button::Size::Medium);
      break;
    case State::TextAndSpeech:setHeading(Ids::TextAndSpeech);
      _sliders.emplace_back(Ids::TextSpeed, getSlotPos(1), true,
                            getUserPreference(PreferenceNames::SayLineSpeed, PreferenceDefaultValues::SayLineSpeed),
                            [this](auto value) {
                              _isDirty = true;
                              setUserPreference(PreferenceNames::SayLineSpeed, value);
                            });
      _checkboxes.emplace_back(Ids::DisplayText, getSlotPos(3), true,
                               getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText),
                               [this](auto value) {
                                 _isDirty = true;
                                 if (!value && !getUserPreference(PreferenceNames::HearVoice,
                                                                  PreferenceDefaultValues::HearVoice)) {
                                   _checkboxes[1].setChecked(true);
                                   setUserPreference(PreferenceNames::HearVoice, true);
                                 }
                                 setUserPreference(PreferenceNames::DisplayText, value);
                               });
      _checkboxes.emplace_back(Ids::HearVoice, getSlotPos(4), true,
                               getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice),
                               [this](auto value) {
                                 _isDirty = true;
                                 if (!value && !getUserPreference(PreferenceNames::DisplayText,
                                                                  PreferenceDefaultValues::DisplayText)) {
                                   _checkboxes[0].setChecked(true);
                                   setUserPreference(PreferenceNames::DisplayText, true);
                                 }
                                 setUserPreference(PreferenceNames::HearVoice, value);
                               });
      _switchButtons.push_back(SwitchButton({Ids::EnglishText, Ids::FrenchText, Ids::ItalianText, Ids::GermanText,
                                             Ids::SpanishText}, getSlotPos(5), true,
                                            getLanguageUserPreference(), [this](auto index) {
            _isDirty = true;
            setUserPreference(PreferenceNames::Language, LanguageValues[index]);
          }));
      _buttons.emplace_back(Ids::Back,
                            getSlotPos(9),
                            [this]() { updateState(State::Main); },
                            true,
                            Button::Size::Medium);
      break;
    case State::Help:setHeading(Ids::Help);
      _buttons.emplace_back(Ids::Introduction, getSlotPos(1), [this]() {
        _pEngine->execute("HelpScreens.helpIntro()");
      }, true);
      _buttons.emplace_back(Ids::MouseTips, getSlotPos(2), []() {}, false);
      _buttons.emplace_back(Ids::ControllerTips, getSlotPos(3), []() {}, false);
      _buttons.emplace_back(Ids::ControllerMap, getSlotPos(4), []() {}, false);
      _buttons.emplace_back(Ids::KeyboardMap, getSlotPos(5), []() {}, false);
      _buttons.emplace_back(Ids::Back,
                            getSlotPos(9),
                            [this]() { updateState(State::Main); },
                            true,
                            Button::Size::Medium);
      break;
    default:updateState(State::Main);
      break;
    }

    for (auto &button : _buttons) {
      button.setEngine(_pEngine);
    }
    for (auto &switchButton : _switchButtons) {
      switchButton.setEngine(_pEngine);
    }
    for (auto &checkbox : _checkboxes) {
      checkbox.setEngine(_pEngine);
      checkbox.setSpriteSheet(&_saveLoadSheet);
    }
    for (auto &slider : _sliders) {
      slider.setEngine(_pEngine);
      slider.setSpriteSheet(&_saveLoadSheet);
    }
  }

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
    if (!pEngine)
      return;

    auto &tm = pEngine->getResourceManager();
    _saveLoadSheet.setTextureManager(&tm);
    _saveLoadSheet.load("SaveLoadSheet");

    const auto &headingFont = _pEngine->getResourceManager().getFntFont("HeadingFont.fnt");
    _headingText.setFont(headingFont);
    _headingText.setColor(ngf::Colors::White);

    _quit.setEngine(pEngine);
    _quit.setCallback([this](bool result) {
      if (result)
        _pEngine->quit();
      _showQuit = result;
    });

    _saveload.setEngine(pEngine);
    _saveload.setCallback([this]() {
      _showSaveLoad = false;
    });
    _saveload.setSlotCallback([this](int slot) {
      if (_saveload.getSaveMode()) {
        _pEngine->saveGame(slot);
        _showSaveLoad = false;
        if (_callback)
          _callback();
      } else {
        _pEngine->loadGame(slot);
        _showSaveLoad = false;
        if (_callback)
          _callback();
      }
    });

    updateState(State::Main);
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) {
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
    auto rect = _saveLoadSheet.getRect("options_background");
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
    _headingText.draw(target, {});

    // controls
    for (auto &button : _buttons) {
      button.draw(target, {});
    }
    for (auto &switchButton : _switchButtons) {
      switchButton.draw(target, {});
    }
    for (auto &checkbox : _checkboxes) {
      checkbox.draw(target, {});
    }
    for (auto &slider : _sliders) {
      slider.draw(target, {});
    }

    target.setView(view);

    if (_showSaveLoad) {
      _saveload.draw(target, states);
    }

    if (_showQuit) {
      _quit.draw(target, states);
    }
  }

  void update(const ngf::TimeSpan &elapsed) {
    if (_showSaveLoad) {
      _saveload.update(elapsed);
      return;
    }

    if (_showQuit) {
      _quit.update(elapsed);
      return;
    }

    auto pos = _pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
      ngf::View(ngf::frect::fromPositionSize({0, 0},{Screen::Width,Screen::Height})));
    for (auto &button : _buttons) {
      button.update(pos);
    }
    for (auto &switchButton : _switchButtons) {
      switchButton.update(pos);
    }
    for (auto &checkbox : _checkboxes) {
      checkbox.update(pos);
    }
    for (auto &slider : _sliders) {
      slider.update(pos);
    }
  }
};

OptionsDialog::OptionsDialog()
    : _pImpl(std::make_unique<Impl>()) {
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::setSaveEnabled(bool enabled) { _pImpl->_saveEnabled = enabled; }

void OptionsDialog::setEngine(Engine *pEngine) { _pImpl->setEngine(pEngine); }

void OptionsDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  _pImpl->draw(target, states);
}

void OptionsDialog::update(const ngf::TimeSpan &elapsed) {
  _pImpl->update(elapsed);
}

void OptionsDialog::showHelp() {
  _pImpl->updateState(Impl::State::Help);
}

void OptionsDialog::setCallback(Callback callback) {
  _pImpl->_callback = std::move(callback);
}
}
