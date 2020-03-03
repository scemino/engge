#include "_Button.hpp"
#include "_Checkbox.hpp"
#include "_Slider.hpp"
#include "_SwitchButton.hpp"
#include "Audio/SoundManager.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Preferences.hpp"
#include "Font/FntFont.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Graphics/Text.hpp"
#include "System/Logger.hpp"
#include "UI/OptionsDialog.hpp"
#include "UI/QuitDialog.hpp"
#include "imgui.h"


namespace ng
{
struct OptionsDialog::Impl
{
    enum class State {Main, Sound, Video, Controls, TextAndSpeech, Help};

    struct Ids {
        inline static const int EnglishText=98001;
        inline static const int FrenchText=98003;
        inline static const int ItalianText=98005;
        inline static const int GermanText=98007;
        inline static const int SpanishText=98009;
        inline static const int Back=99904;
        inline static const int LoadGame = 99910;
        inline static const int SaveGame=99911;
        inline static const int NewGame=99912;
        inline static const int Options=99913;
        inline static const int Credits=99914;
        inline static const int Quit=99915;
        inline static const int Sound=99916;
        inline static const int Video=99917;
        inline static const int Controls=99918;
        inline static const int TextAndSpeech=99919;
        inline static const int Fullscreen=99927;
        inline static const int SafeArea=99929;
        inline static const int RetroFonts=99933;
        inline static const int RetroVerbs=99934;
        inline static const int ClassicSentence=99935;
        inline static const int SoundVolume=99937;
        inline static const int MusicVolume=99938;
        inline static const int VoiceVolume=99939;
        inline static const int Controller=99940;
        inline static const int TextSpeed=99941;
        inline static const int DisplayText=99942;
        inline static const int HearVoice=99943;
        inline static const int ScrollSyncCursor=99960;
        inline static const int Help=99961;
        inline static const int InvertVerbColors=99964;
        inline static const int ToiletPaperOver=99965;
        inline static const int Introduction=99966;
        inline static const int MouseTips=99967;
        inline static const int ControllerTips=99968;
        inline static const int ControllerMap=99969;
    };    

    inline static const std::array<std::string,5> LanguageValues = {"en","fr","it","de","es"};
    static constexpr float yPosLarge = 58.f;
    static constexpr float yPosSmall = 54.f;

    Engine* _pEngine{nullptr};
    SpriteSheet _saveLoadSheet;

    Text _headingText;
    std::vector<_Button> _buttons;
    std::vector<_SwitchButton> _switchButtons;
    std::vector<_Checkbox> _checkboxes;
    std::vector<_Slider> _sliders;
    bool _showQuit{false};
    QuitDialog _quit;
    Callback _callback{nullptr};
    bool _isDirty{false};

    inline static float getSlotPos(int slot)
    {
        return 44.f+yPosLarge+yPosSmall*slot;
    }

    void setHeading(int id)
    {
        _headingText.setString(_pEngine->getText(id));
        auto textRect = _headingText.getGlobalBounds();
        _headingText.setPosition(sf::Vector2f((Screen::Width-textRect.width)/2.f, 44.f-textRect.height/2));
    }

    template <typename T>
    void setUserPreference(const std::string &name, T value)
    {
        Locator<Preferences>::get().setUserPreference(name, value);
    }

    template <typename T>
    T getUserPreference(const std::string &name, T value) const
    {
        return Locator<Preferences>::get().getUserPreference(name, value);
    }

    int getLanguageUserPreference() const
    {
        auto lang = Locator<Preferences>::get().getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
        auto it = std::find(LanguageValues.begin(),LanguageValues.end(),lang);
        return static_cast<int>(std::distance(LanguageValues.begin(),it));
    }

    void updateState(State state)
    {
        if(_isDirty)
        {
            Locator<Preferences>::get().save();
            _isDirty = false;
        }
        _sliders.clear();
        _buttons.clear();
        _switchButtons.clear();
        _checkboxes.clear();
        switch(state)
        {
        case State::Main:
            setHeading(Ids::Options);
            _buttons.emplace_back(Ids::SaveGame, getSlotPos(0), [](){}, false);
            _buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [](){}, false);
            _buttons.emplace_back(Ids::Sound, getSlotPos(2), [this](){ updateState(State::Sound); });
            _buttons.emplace_back(Ids::Video, getSlotPos(3), [this](){ updateState(State::Video); });
            _buttons.emplace_back(Ids::Controls, getSlotPos(4), [this](){ updateState(State::Controls); });
            _buttons.emplace_back(Ids::TextAndSpeech, getSlotPos(5), [this](){ updateState(State::TextAndSpeech); });
            _buttons.emplace_back(Ids::Help, getSlotPos(6), [this](){ updateState(State::Help); });
            _buttons.emplace_back(Ids::Quit, getSlotPos(7), [this](){ _showQuit = true;}, true);
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ if(_callback) _callback(); }, true, _Button::Size::Medium);
            break;
        case State::Sound:
            setHeading(Ids::Sound);
            _sliders.emplace_back(Ids::SoundVolume, getSlotPos(2), true, Locator<SoundManager>::get().getSoundVolume(), [this](auto value){ _isDirty = true; Locator<SoundManager>::get().setSoundVolume(value); });
            _sliders.emplace_back(Ids::MusicVolume, getSlotPos(3), true, Locator<SoundManager>::get().getMusicVolume(), [this](auto value){ _isDirty = true; Locator<SoundManager>::get().setMusicVolume(value); });
            _sliders.emplace_back(Ids::VoiceVolume, getSlotPos(4), true, Locator<SoundManager>::get().getTalkVolume(), [this](auto value){ _isDirty = true; Locator<SoundManager>::get().setTalkVolume(value); });
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); }, true, _Button::Size::Medium);
            break;
        case State::Video:
            setHeading(Ids::Video);
            _checkboxes.emplace_back(Ids::Fullscreen, getSlotPos(1), false, 
                getUserPreference(PreferenceNames::Fullscreen, PreferenceDefaultValues::Fullscreen),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::Fullscreen, value); });
            _sliders.emplace_back(Ids::SafeArea, getSlotPos(2), false, 
                getUserPreference(PreferenceNames::SafeArea, PreferenceDefaultValues::SafeArea),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::SafeArea, value); });
            _checkboxes.emplace_back(Ids::ToiletPaperOver, getSlotPos(4), true, 
                getUserPreference(PreferenceNames::ToiletPaperOver, PreferenceDefaultValues::ToiletPaperOver),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::ToiletPaperOver, value); });
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); }, true, _Button::Size::Medium);
            break;
        case State::Controls:
            setHeading(Ids::Controls);
            _checkboxes.emplace_back(Ids::Controller, getSlotPos(1), false, 
                getUserPreference(PreferenceNames::Controller, PreferenceDefaultValues::Controller),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::Controller, value); });
            _checkboxes.emplace_back(Ids::ScrollSyncCursor, getSlotPos(2), false, 
                getUserPreference(PreferenceNames::ScrollSyncCursor, PreferenceDefaultValues::ScrollSyncCursor),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::ScrollSyncCursor, value); });
            _checkboxes.emplace_back(Ids::InvertVerbColors, getSlotPos(4), true, 
                getUserPreference(PreferenceNames::InvertVerbHighlight, PreferenceDefaultValues::InvertVerbHighlight),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::InvertVerbHighlight, value); });
            _checkboxes.emplace_back(Ids::RetroFonts, getSlotPos(5), true, 
                getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::RetroFonts, value); });
            _checkboxes.emplace_back(Ids::RetroVerbs, getSlotPos(6), true, 
                getUserPreference(PreferenceNames::RetroVerbs, PreferenceDefaultValues::RetroVerbs),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::RetroVerbs, value); });
            _checkboxes.emplace_back(Ids::ClassicSentence, getSlotPos(7), true, 
                getUserPreference(PreferenceNames::ClassicSentence, PreferenceDefaultValues::ClassicSentence),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::ClassicSentence, value); });
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); }, true, _Button::Size::Medium);
            break;
        case State::TextAndSpeech:
            setHeading(Ids::TextAndSpeech);
            _sliders.emplace_back(Ids::TextSpeed, getSlotPos(1), true, 
                getUserPreference(PreferenceNames::SayLineSpeed, PreferenceDefaultValues::SayLineSpeed),
                [this](auto value){ _isDirty = true; setUserPreference(PreferenceNames::SayLineSpeed, value); });
            _checkboxes.emplace_back(Ids::DisplayText, getSlotPos(3), true, 
                getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText),
                [this](auto value) { 
                    _isDirty = true;
                    if(!value && !getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice)) {
                        _checkboxes[1].setChecked(true);
                        setUserPreference(PreferenceNames::HearVoice, true);    
                    }
                    setUserPreference(PreferenceNames::DisplayText, value);
                });
            _checkboxes.emplace_back(Ids::HearVoice, getSlotPos(4), true, 
                getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice),
                [this](auto value) {
                    _isDirty = true;
                    if(!value && !getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText)) {
                        _checkboxes[0].setChecked(true);
                        setUserPreference(PreferenceNames::DisplayText, true);    
                    }
                    setUserPreference(PreferenceNames::HearVoice, value);
                });
            _switchButtons.push_back(_SwitchButton({Ids::EnglishText, Ids::FrenchText, Ids::ItalianText, Ids::GermanText, Ids::SpanishText}, getSlotPos(5), true, 
                getLanguageUserPreference(), [this](auto index){
                _isDirty = true; setUserPreference(PreferenceNames::Language, LanguageValues[index]);
            }));
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); }, true, _Button::Size::Medium);
            break;
        case State::Help:
            setHeading(Ids::Help);
            _buttons.emplace_back(Ids::Introduction, getSlotPos(1), [](){}, false);
            _buttons.emplace_back(Ids::MouseTips, getSlotPos(2), [](){}, false);
            _buttons.emplace_back(Ids::ControllerTips, getSlotPos(3), [](){}, false);
            _buttons.emplace_back(Ids::ControllerMap, getSlotPos(4), [](){}, false);
            // _buttons.emplace_back(Ids::KeyboardMap, getSlotPos(5), [this](){}, false);
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); }, true, _Button::Size::Medium);
            break;
        default:
            updateState(State::Main);
            break;
        }

        for(auto& button : _buttons) {
            button.setEngine(_pEngine);
        }
        for(auto& switchButton : _switchButtons) {
            switchButton.setEngine(_pEngine);
        }
        for(auto& checkbox : _checkboxes) {
            checkbox.setEngine(_pEngine);
            checkbox.setSpriteSheet(&_saveLoadSheet);
        }
        for(auto& slider : _sliders) {
            slider.setEngine(_pEngine);
            slider.setSpriteSheet(&_saveLoadSheet);
        }
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;
        if(!pEngine) return;

        TextureManager& tm = pEngine->getTextureManager();
        _saveLoadSheet.setTextureManager(&tm);
        _saveLoadSheet.load("SaveLoadSheet");

        const FntFont& headingFont = _pEngine->getTextureManager().getFntFont("HeadingFont.fnt");
        _headingText.setFont(headingFont);
        _headingText.setFillColor(sf::Color::White);

        _quit.setEngine(pEngine);
        _quit.setCallback([this](bool result){
            if(result) _pEngine->quit();
            _showQuit = result;
        });

        updateState(State::Main);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states)
    {
        const auto view = target.getView();
        auto viewRect = sf::FloatRect(0, 0, 320, 180);
        target.setView(sf::View(viewRect));

        sf::Color backColor{0,0,0,128};
        sf::RectangleShape fadeShape;
        fadeShape.setSize(sf::Vector2f(viewRect.width, viewRect.height));
        fadeShape.setFillColor(backColor);
        target.draw(fadeShape);
        
        // draw background
        auto viewCenter = sf::Vector2f(viewRect.width/2,viewRect.height/2);
        auto rect = _saveLoadSheet.getRect("options_background");
        sf::Sprite sprite;
        sprite.setPosition(viewCenter);
        sprite.setTexture(_saveLoadSheet.getTexture());
        sprite.setOrigin(rect.width/2,rect.height/2);
        sprite.setTextureRect(rect);
        target.draw(sprite);

        viewRect = sf::FloatRect(0, 0, Screen::Width, Screen::Height);
        target.setView(sf::View(viewRect));

        // heading
        target.draw(_headingText);

        // controls
        for(auto& button : _buttons) {
            target.draw(button);
        }
        for(auto& switchButton : _switchButtons) {
            target.draw(switchButton);
        }
        for(auto& checkbox : _checkboxes) {
            target.draw(checkbox);
        }
        for(auto& slider : _sliders) {
            target.draw(slider);
        }
        
        target.setView(view);

        if(_showQuit) {
            target.draw(_quit, states);
        }
    }

    void update(const sf::Time& elapsed)
    {
        if(_showQuit) {
            _quit.update(elapsed);
            return;
        }

        auto pos = (sf::Vector2f)_pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()), sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));
        for(auto& button : _buttons) {
            button.update(pos);
        }
        for(auto& switchButton : _switchButtons) {
            switchButton.update(pos);
        }
        for(auto& checkbox : _checkboxes) {
            checkbox.update(pos);
        }
         for(auto& slider : _sliders) {
            slider.update(pos);
        }
    }
};

OptionsDialog::OptionsDialog()
: _pImpl(std::make_unique<Impl>())
{
}

OptionsDialog::~OptionsDialog() = default;

void OptionsDialog::setEngine(Engine* pEngine) { _pImpl->setEngine(pEngine); }

void OptionsDialog::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    _pImpl->draw(target, states);
}

void OptionsDialog::update(const sf::Time& elapsed)
{
    _pImpl->update(elapsed);
}

void OptionsDialog::showHelp()
{
    _pImpl->updateState(Impl::State::Help);
}

void OptionsDialog::setCallback(Callback callback)
{
    _pImpl->_callback = callback;
}
}
