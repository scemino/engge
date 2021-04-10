#include "Button.hpp"
#include "Checkbox.hpp"
#include "Slider.hpp"
#include "SwitchButton.hpp"
#include <engge/EnggeApplication.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/System/Logger.hpp>
#include <engge/UI/OptionsDialog.hpp>
#include <engge/UI/SaveLoadDialog.hpp>
#include <engge/UI/QuitDialog.hpp>
#include "HelpDialog.hpp"
#include <utility>
#include <ngf/Graphics/FntFont.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/System/Mouse.h>

namespace ng {
struct OptionsDialog::Impl {
  enum class State { None, Main, Sound, Video, Controls, TextAndSpeech, Help };

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

  Engine *m_pEngine{nullptr};
  SpriteSheet m_saveLoadSheet;

  ng::Text m_headingText;
  std::vector<Button> m_buttons;
  std::vector<SwitchButton> m_switchButtons;
  std::vector<Checkbox> m_checkboxes;
  std::vector<Slider> m_sliders;
  bool m_showQuit{false};
  bool m_showSaveLoad{false};
  bool m_showHelp{false};
  QuitDialog m_quitDialog;
  SaveLoadDialog m_saveLoadDialog;
  HelpDialog m_help;
  Callback m_callback{nullptr};
  bool m_isDirty{false};
  State m_state{State::None};
  State m_nextState{State::None};
  bool m_saveEnabled{false};

  inline static float getSlotPos(int slot) {
    return yPosStart + yPosLarge + yPosSmall * static_cast<float>(slot);
  }

  void setHeading(int id) {
    m_headingText.setWideString(Engine::getText(id));
    auto textRect = m_headingText.getLocalBounds();
    m_headingText.getTransform().setPosition({(Screen::Width - textRect.getWidth()) / 2.f,
                                              yPosStart - textRect.getHeight() / 2});
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

  void setState(State state) {
    m_nextState = state;
  }

  void onStateChanged() {
    if (m_isDirty) {
      Locator<Preferences>::get().save();
      m_quitDialog.updateLanguage();
      m_isDirty = false;
    }
    m_sliders.clear();
    m_buttons.clear();
    m_switchButtons.clear();
    m_checkboxes.clear();
    switch (m_state) {
    case State::Main:setHeading(Ids::Options);
      m_buttons.emplace_back(Ids::SaveGame, getSlotPos(0), [this]() {
        m_saveLoadDialog.updateLanguage();
        m_saveLoadDialog.setSaveMode(true);
        m_showSaveLoad = true;
      }, m_saveEnabled);
      m_buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this]() {
        m_saveLoadDialog.updateLanguage();
        m_saveLoadDialog.setSaveMode(false);
        m_showSaveLoad = true;
      });
      m_buttons.emplace_back(Ids::Sound, getSlotPos(2), [this]() { setState(State::Sound); });
      m_buttons.emplace_back(Ids::Video, getSlotPos(3), [this]() { setState(State::Video); });
      m_buttons.emplace_back(Ids::Controls, getSlotPos(4), [this]() { setState(State::Controls); });
      m_buttons.emplace_back(Ids::TextAndSpeech, getSlotPos(5), [this]() { setState(State::TextAndSpeech); });
      m_buttons.emplace_back(Ids::Help, getSlotPos(6), [this]() { setState(State::Help); });
      m_buttons.emplace_back(Ids::Quit, getSlotPos(7), [this]() { m_showQuit = true; }, true);
      m_buttons.emplace_back(Ids::Back, getSlotPos(9), [this]() {
        if (m_callback)
          m_callback();
      }, true, Button::Size::Medium);
      break;
    case State::Sound:setHeading(Ids::Sound);
      m_sliders.emplace_back(Ids::SoundVolume,
                             getSlotPos(2),
                             true,
                             Locator<SoundManager>::get().getSoundVolume(),
                             [this](auto value) {
                               m_isDirty = true;
                              Locator<SoundManager>::get().setSoundVolume(value);
                            });
      m_sliders.emplace_back(Ids::MusicVolume,
                             getSlotPos(3),
                             true,
                             Locator<SoundManager>::get().getMusicVolume(),
                             [this](auto value) {
                               m_isDirty = true;
                              Locator<SoundManager>::get().setMusicVolume(value);
                            });
      m_sliders.emplace_back(Ids::VoiceVolume,
                             getSlotPos(4),
                             true,
                             Locator<SoundManager>::get().getTalkVolume(),
                             [this](auto value) {
                               m_isDirty = true;
                              Locator<SoundManager>::get().setTalkVolume(value);
                            });
      m_buttons.emplace_back(Ids::Back,
                             getSlotPos(9),
                             [this]() { setState(State::Main); },
                             true,
                             Button::Size::Medium);
      break;
    case State::Video:setHeading(Ids::Video);
      m_checkboxes.emplace_back(Ids::Fullscreen, getSlotPos(1), true,
                                getUserPreference(PreferenceNames::Fullscreen, PreferenceDefaultValues::Fullscreen),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::Fullscreen, value);
                               });
      m_sliders.emplace_back(Ids::SafeArea, getSlotPos(2), false,
                             getUserPreference(PreferenceNames::SafeArea, PreferenceDefaultValues::SafeArea),
                             [this](auto value) {
                               m_isDirty = true;
                              setUserPreference(PreferenceNames::SafeArea, value);
                            });
      m_checkboxes.emplace_back(Ids::ToiletPaperOver, getSlotPos(4), true,
                                getUserPreference(PreferenceNames::ToiletPaperOver,
                                                 PreferenceDefaultValues::ToiletPaperOver),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::ToiletPaperOver, value);
                                 ScriptEngine::call("setSettingVar", "toilet_paper_over", value ? 1 : 0);
                               });
      m_checkboxes.emplace_back(Ids::AnnoyingInJokes, getSlotPos(5), true,
                                getUserPreference(PreferenceNames::AnnoyingInJokes,
                                                 PreferenceDefaultValues::AnnoyingInJokes),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::AnnoyingInJokes, value);
                                 ScriptEngine::call("setSettingVar", "annoying_injokes", value ? 1 : 0);
                               });
      m_buttons.emplace_back(Ids::Back,
                             getSlotPos(9),
                             [this]() { setState(State::Main); },
                             true,
                             Button::Size::Medium);
      break;
    case State::Controls:setHeading(Ids::Controls);
      m_checkboxes.emplace_back(Ids::Controller, getSlotPos(1), false,
                                getUserPreference(PreferenceNames::Controller, PreferenceDefaultValues::Controller),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::Controller, value);
                               });
      m_checkboxes.emplace_back(Ids::ScrollSyncCursor, getSlotPos(2), false,
                                getUserPreference(PreferenceNames::ScrollSyncCursor,
                                                 PreferenceDefaultValues::ScrollSyncCursor),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::ScrollSyncCursor, value);
                               });
      m_checkboxes.emplace_back(Ids::InvertVerbColors, getSlotPos(4), true,
                                getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                 PreferenceDefaultValues::InvertVerbHighlight),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::InvertVerbHighlight, value);
                               });
      m_checkboxes.emplace_back(Ids::RetroFonts, getSlotPos(5), true,
                                getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::RetroFonts, value);
                               });
      m_checkboxes.emplace_back(Ids::RetroVerbs, getSlotPos(6), true,
                                getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::RetroVerbs, value);
                                 if (value) {
                                   m_checkboxes[3].setChecked(true);
                                   m_checkboxes[5].setChecked(true);
                                 }
                               });
      m_checkboxes.emplace_back(Ids::ClassicSentence, getSlotPos(7), true,
                                getUserPreference(PreferenceNames::ClassicSentence,
                                                 PreferenceDefaultValues::ClassicSentence),
                                [this](auto value) {
                                  m_isDirty = true;
                                 setUserPreference(PreferenceNames::ClassicSentence, value);
                               });
      m_buttons.emplace_back(Ids::Back,
                             getSlotPos(9),
                             [this]() { setState(State::Main); },
                             true,
                             Button::Size::Medium);
      break;
    case State::TextAndSpeech:setHeading(Ids::TextAndSpeech);
      m_sliders.emplace_back(Ids::TextSpeed, getSlotPos(1), true,
                             getUserPreference(PreferenceNames::SayLineSpeed, PreferenceDefaultValues::SayLineSpeed),
                             [this](auto value) {
                               m_isDirty = true;
                              setUserPreference(PreferenceNames::SayLineSpeed, value);
                            });
      m_checkboxes.emplace_back(Ids::DisplayText, getSlotPos(3), true,
                                getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText),
                                [this](auto value) {
                                  m_isDirty = true;
                                 if (!value && !getUserPreference(PreferenceNames::HearVoice,
                                                                  PreferenceDefaultValues::HearVoice)) {
                                   m_checkboxes[1].setChecked(true);
                                   setUserPreference(PreferenceNames::HearVoice, true);
                                 }
                                 setUserPreference(PreferenceNames::DisplayText, value);
                               });
      m_checkboxes.emplace_back(Ids::HearVoice, getSlotPos(4), true,
                                getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice),
                                [this](auto value) {
                                  m_isDirty = true;
                                 if (!value && !getUserPreference(PreferenceNames::DisplayText,
                                                                  PreferenceDefaultValues::DisplayText)) {
                                   m_checkboxes[0].setChecked(true);
                                   setUserPreference(PreferenceNames::DisplayText, true);
                                 }
                                 setUserPreference(PreferenceNames::HearVoice, value);
                               });
      m_switchButtons.push_back(SwitchButton({Ids::EnglishText, Ids::FrenchText, Ids::ItalianText, Ids::GermanText,
                                              Ids::SpanishText}, getSlotPos(5), true,
                                             getLanguageUserPreference(), [this](auto index) {
            m_isDirty = true;
            setUserPreference(PreferenceNames::Language, LanguageValues[index]);
          }));
      m_buttons.emplace_back(Ids::Back,
                             getSlotPos(9),
                             [this]() { setState(State::Main); },
                             true,
                             Button::Size::Medium);
      break;
    case State::Help:setHeading(Ids::Help);
      m_buttons.emplace_back(Ids::Introduction, getSlotPos(1), [this]() {
        m_help.init(m_pEngine, [this]() { m_showHelp = false; }, {1, 2, 3, 4, 5, 6});
        m_showHelp = true;
      }, true);
      m_buttons.emplace_back(Ids::MouseTips, getSlotPos(2), [this]() {
        m_help.init(m_pEngine, [this]() { m_showHelp = false; }, {7, 8, 9});
        m_showHelp = true;
      }, true);
      m_buttons.emplace_back(Ids::ControllerTips, getSlotPos(3), [this]() {
        m_help.init(m_pEngine, [this]() { m_showHelp = false; }, {10, 11, 12, 13, 14});
        m_showHelp = true;
      }, true);
      m_buttons.emplace_back(Ids::ControllerMap, getSlotPos(4), [this]() {
        m_help.init(m_pEngine, [this]() { m_showHelp = false; }, {15});
        m_showHelp = true;
      }, true);
      m_buttons.emplace_back(Ids::KeyboardMap, getSlotPos(5), [this]() {
        m_help.init(m_pEngine, [this]() { m_showHelp = false; }, {16});
        m_showHelp = true;
      }, true);
      m_buttons.emplace_back(Ids::Back,
                             getSlotPos(9),
                             [this]() { setState(State::Main); },
                             true,
                             Button::Size::Medium);
      break;
    default:setState(State::Main);
      break;
    }

    for (auto &button : m_buttons) {
      button.setEngine(m_pEngine);
    }
    for (auto &switchButton : m_switchButtons) {
      switchButton.setEngine(m_pEngine);
    }
    for (auto &checkbox : m_checkboxes) {
      checkbox.setEngine(m_pEngine);
      checkbox.setSpriteSheet(&m_saveLoadSheet);
    }
    for (auto &slider : m_sliders) {
      slider.setEngine(m_pEngine);
      slider.setSpriteSheet(&m_saveLoadSheet);
    }
  }

  void setEngine(Engine *pEngine) {
    m_pEngine = pEngine;
    if (!pEngine)
      return;

    auto &tm = pEngine->getResourceManager();
    m_saveLoadSheet.setTextureManager(&tm);
    m_saveLoadSheet.load("SaveLoadSheet");

    const auto &headingFont = m_pEngine->getResourceManager().getFntFont("HeadingFont.fnt");
    m_headingText.setFont(headingFont);
    m_headingText.setColor(ngf::Colors::White);

    m_quitDialog.setEngine(pEngine);
    m_quitDialog.setCallback([this](bool result) {
      if (result)
        m_pEngine->quit();
      m_showQuit = result;
    });

    m_saveLoadDialog.setEngine(pEngine);
    m_saveLoadDialog.setCallback([this]() {
      m_showSaveLoad = false;
    });
    m_saveLoadDialog.setSlotCallback([this](int slot) {
      if (m_saveLoadDialog.getSaveMode()) {
        m_pEngine->saveGame(slot);
        m_showSaveLoad = false;
        if (m_callback)
          m_callback();
      } else {
        m_pEngine->loadGame(slot);
        m_showSaveLoad = false;
        if (m_callback)
          m_callback();
      }
    });

    setState(State::Main);
  }

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) {
    if (m_showHelp) {
      m_help.draw(target, states);
      return;
    }

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
    auto rect = m_saveLoadSheet.getRect("options_background");
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

    // controls
    for (auto &button : m_buttons) {
      button.draw(target, {});
    }
    for (auto &switchButton : m_switchButtons) {
      switchButton.draw(target, {});
    }
    for (auto &checkbox : m_checkboxes) {
      checkbox.draw(target, {});
    }
    for (auto &slider : m_sliders) {
      slider.draw(target, {});
    }

    target.setView(view);

    if (m_showSaveLoad) {
      m_saveLoadDialog.draw(target, states);
    }

    if (m_showQuit) {
      m_quitDialog.draw(target, states);
    }
  }

  void update(const ngf::TimeSpan &elapsed) {
    if (m_state != m_nextState) {
      m_state = m_nextState;
      onStateChanged();
    }

    if (m_showHelp) {
      m_help.update(elapsed);
      return;
    }

    if (m_showSaveLoad) {
      m_saveLoadDialog.update(elapsed);
      return;
    }

    if (m_showQuit) {
      m_quitDialog.update(elapsed);
      return;
    }

    auto pos = m_pEngine->getApplication()->getRenderTarget()->mapPixelToCoords(ngf::Mouse::getPosition(),
                                                                                ngf::View(ngf::frect::fromPositionSize({0,
                                                                                                                       0},
                                                                                                                      {Screen::Width,
                                                                                                                       Screen::Height})));
    for (auto &button : m_buttons) {
      button.update(elapsed, pos);
    }
    for (auto &switchButton : m_switchButtons) {
      switchButton.update(elapsed, pos);
    }
    for (auto &checkbox : m_checkboxes) {
      checkbox.update(elapsed, pos);
    }
    for (auto &slider : m_sliders) {
      slider.update(elapsed, pos);
    }
  }
};

OptionsDialog::OptionsDialog()
    : m_pImpl(std::make_unique<Impl>()) {
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::setSaveEnabled(bool enabled) { m_pImpl->m_saveEnabled = enabled; }

void OptionsDialog::setEngine(Engine *pEngine) { m_pImpl->setEngine(pEngine); }

void OptionsDialog::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  m_pImpl->draw(target, states);
}

void OptionsDialog::update(const ngf::TimeSpan &elapsed) {
  m_pImpl->update(elapsed);
}

void OptionsDialog::showHelp() {
  m_pImpl->setState(Impl::State::Help);
}

void OptionsDialog::setCallback(Callback callback) {
  m_pImpl->m_callback = std::move(callback);
}
}
