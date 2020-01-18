#include "Engine.h"
#include "FntFont.h"
#include "OptionsDialog.h"
#include "Screen.h"
#include "SpriteSheet.h"
#include "Text.h"
#include "Logger.h"
#include "imgui.h"


namespace ng
{

class Button: public sf::Drawable
{
public:
    typedef std::function<void()> Callback;

public:
    Button(int id, float y, Callback callback, bool enabled = true)
    : _id(id), _y(y), _callback(std::move(callback)), _isEnabled(enabled)
    {
    }

    void setCallback(Callback callback)
    {
        _callback=std::move(callback);
    }

    int getId() const { return _id; }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontLarge = _pEngine->getTextureManager().getFntFont("UIFontLarge.fnt");
        text.setFont(uiFontLarge);
        text.setString(_pEngine->getText(_id));
        auto textRect = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(textRect.width/2.f, 0));
        text.setPosition(sf::Vector2f(Screen::Width/2.f, _y));
    }

    void update()
    {
        auto textRect = text.getGlobalBounds();
        auto pos = _pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()));

        sf::Color color;
        if(!_isEnabled)
        {
            color=_disabledColor;
        }
        else if(textRect.contains((sf::Vector2f)pos))
        {
            color=_hoveColor;
            bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            ImGuiIO &io = ImGui::GetIO();
            if(!io.WantCaptureMouse && _wasMouseDown && !isDown)
            {
                _callback();
            }
            _wasMouseDown = isDown;
        }
        else
        {
            color=sf::Color::White;
        }
        text.setFillColor(color);
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
    }

private:
    Engine* _pEngine{nullptr};
    int _id{0};
    bool _isEnabled{true};
    float _y{0};
    bool _isOver{false};
    bool _wasMouseDown{false};
    Callback _callback;
    Text text;
    inline static const sf::Color _disabledColor{255,255,255,128};
    inline static const sf::Color _hoveColor{sf::Color::Yellow};
};

class SwitchButton: public sf::Drawable
{
public:
    typedef std::function<void()>		Callback;

public:
    SwitchButton(std::initializer_list<int> ids, float y, bool enabled = true)
    : _ids(ids), _y(y), _isEnabled(enabled)
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontLarge = _pEngine->getTextureManager().getFntFont("UIFontLarge.fnt");
        text.setFont(uiFontLarge);
        text.setString(_pEngine->getText(_ids[_index]));
        auto textRect = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(textRect.width/2.f, 0));
        text.setPosition(sf::Vector2f(Screen::Width/2.f, _y));
    }

    void update()
    {
        auto textRect = text.getGlobalBounds();
        auto pos = _pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()));

        sf::Color color;
        if(!_isEnabled)
        {
            color=_disabledColor;
        }
        else if(textRect.contains((sf::Vector2f)pos))
        {
            color=_hoveColor;
            bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            ImGuiIO &io = ImGui::GetIO();
            if(!io.WantCaptureMouse && _wasMouseDown && !isDown)
            {
                _index=(_index+1)%_ids.size();
                text.setString(_pEngine->getText(_ids[_index]));
                auto textRect = text.getLocalBounds();
                text.setOrigin(sf::Vector2f(textRect.width/2.f, 0));
            }
            _wasMouseDown = isDown;
        }
        else
        {
            color=sf::Color::White;
        }
        text.setFillColor(color);
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
    }

private:
    Engine* _pEngine{nullptr};
    std::vector<int> _ids;
    int _index{0};
    bool _isEnabled{true};
    float _y{0};
    bool _isOver{false};
    bool _wasMouseDown{false};
    Callback _callback;
    Text text;
    inline static const sf::Color _disabledColor{255,255,255,128};
    inline static const sf::Color _hoveColor{sf::Color::Yellow};
};

class Checkbox: public sf::Drawable
{
public:
    typedef std::function<void(bool)> Callback;

public:
    Checkbox(int id, float y, bool enabled = true, bool checked = false)
    : _id(id), _y(y), _isEnabled(enabled), _isChecked(checked)
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontMedium = _pEngine->getTextureManager().getFntFont("UIFontMedium.fnt");
        _text.setFont(uiFontMedium);
        _text.setString(_pEngine->getText(_id));
        auto textRect = _text.getLocalBounds();
        _text.setOrigin(sf::Vector2f(0, textRect.height));
        _text.setPosition(420.f, _y);
    }

    void setSpriteSheet(SpriteSheet* pSpriteSheet)
    {
        _pSpriteSheet=pSpriteSheet;
        auto checkedRect = pSpriteSheet->getRect("option_unchecked");
        _sprite.setPosition(820.f, _y);
        sf::Vector2f scale(Screen::Width/320.f,Screen::Height/180.f);
        _sprite.setScale(scale);
        _sprite.setOrigin(checkedRect.width/2.f, checkedRect.height/2.f);
        _sprite.setTexture(pSpriteSheet->getTexture());
        _sprite.setTextureRect(checkedRect);
    }

    void setChecked(bool checked) {
        if(_isChecked!=checked) {
            _isChecked = checked;
            if(onValueChanged) {
                onValueChanged.value()(_isChecked);
            }
        }
    }
    
    inline bool isChecked() const { return _isChecked; }

    void setCallback(Callback callback)
    {
        onValueChanged=callback;
    }

    void update(sf::Vector2f pos)
    {
        auto textRect = _sprite.getGlobalBounds();
        
        sf::Color color;
        if(!_isEnabled)
        {
            color=_disabledColor;
        }
        else if(textRect.contains(pos))
        {
            color = _hoveColor;
            bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            ImGuiIO &io = ImGui::GetIO();
            if(!io.WantCaptureMouse && _wasMouseDown && !isDown)
            {
                setChecked(!_isChecked);
            }
            _wasMouseDown = isDown;
        }
        else
        {
            color=sf::Color::White;
        }
        _sprite.setColor(color);
        _text.setFillColor(color);

        auto checkedRect = _isChecked ? _pSpriteSheet->getRect("option_checked"):_pSpriteSheet->getRect("option_unchecked");
        _sprite.setTextureRect(checkedRect);
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(_text, states);
        target.draw(_sprite, states);
    }

private:
    Engine* _pEngine{nullptr};
    int _id{0};
    bool _isEnabled{true};
    float _y{0};
    bool _isOver{false};
    bool _isChecked{false};
    bool _wasMouseDown{false};
    Callback _callback;
    Text _text;
    sf::Sprite _sprite;
    SpriteSheet* _pSpriteSheet{nullptr};
    std::optional<Callback> onValueChanged;
    inline static const sf::Color _disabledColor{255,255,255,128};
    inline static const sf::Color _hoveColor{sf::Color::Yellow};
};

class Slider: public sf::Drawable
{
public:
    typedef std::function<void(float)> Callback;

    Slider(float y, bool enabled = true)
    : _y(y), _isEnabled(enabled)
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;
    }

    void setCallback(Callback callback)
    {
        onValueChanged=callback;
    }

    inline float getValue() const { return _value;}

    void setSpriteSheet(SpriteSheet* pSpriteSheet)
    {
        _pSpriteSheet=pSpriteSheet;
        auto sliderRect = pSpriteSheet->getRect("slider");
        auto handleRect = pSpriteSheet->getRect("slider_handle");
        sf::Vector2f scale(Screen::Width/320.f,Screen::Height/180.f);
        _sprite.setPosition(Screen::Width/2.f, _y);
        _sprite.setScale(scale);
        _sprite.setOrigin(sliderRect.width/2.f, sliderRect.height/2.f);
        _sprite.setTexture(pSpriteSheet->getTexture());
        _sprite.setTextureRect(sliderRect);

        _spriteHandle.setPosition(Screen::Width/2.f-(sliderRect.width*scale.x/2.f), _y);
        _spriteHandle.setScale(scale);
        _spriteHandle.setOrigin(handleRect.width/2.f, handleRect.height/2.f);
        _spriteHandle.setTexture(pSpriteSheet->getTexture());
        _spriteHandle.setTextureRect(handleRect);

        _min = Screen::Width/2.f-(sliderRect.width*scale.x/2.f);
        _max = Screen::Width/2.f+(sliderRect.width*scale.x/2.f);
    }

    void update(sf::Vector2f pos)
    {
        auto textRect = _sprite.getGlobalBounds();
        bool isDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        if(!isDown){
            _isDragging=false;
        }
        sf::Color color;
        if(!_isEnabled)
        {
            color=_disabledColor;
        }
        else if(textRect.contains(pos))
        {
            color=_hoveColor;
            ImGuiIO &io = ImGui::GetIO();
            if(!io.WantCaptureMouse && isDown)
            {
                _isDragging = true;
            }
            _wasMouseDown = isDown;
        }
        else
        {
            color=sf::Color::White;
        }
        _sprite.setColor(color);

        if(_isDragging){
            auto x = std::clamp(pos.x, _min, _max);
            auto value = (x-_min)/(_max-_min);
            if(_value!=value){
                _value=value;
                if(onValueChanged) {
                    onValueChanged.value()(value);
                }
            }
            _spriteHandle.setPosition(x, _y);
        }
    }

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(_sprite, states);
        target.draw(_spriteHandle, states);
    }

private:
    Engine* _pEngine{nullptr};
    int _id{0};
    bool _isEnabled{true};
    float _y{0};
    float _min{0}, _max{0}, _value{0};
    bool _isOver{false};
    bool _isDragging{false};
    bool _wasMouseDown{false};
    sf::Sprite _sprite;
    sf::Sprite _spriteHandle;
    SpriteSheet* _pSpriteSheet{nullptr};
    std::optional<Callback> onValueChanged;
    inline static const sf::Color _disabledColor{255,255,255,128};
    inline static const sf::Color _hoveColor{sf::Color::Yellow};
};

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
        inline static const int Controller=99940;
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

    Engine* _pEngine{nullptr};
    SpriteSheet _saveLoadSheet;

    Text _headingText;
    std::vector<Button> _buttons;
    std::vector<SwitchButton> _switchButtons;
    std::vector<Checkbox> _checkboxes;
    std::vector<Slider> _sliders;
    static constexpr float yPosLarge = 58.f;
    static constexpr float yPosSmall = 54.f;

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

    void updateState(State state)
    {
        _sliders.clear();
        _buttons.clear();
        _switchButtons.clear();
        _checkboxes.clear();
        switch(state)
        {
        case State::Main:
            setHeading(Ids::Options);
            _buttons.emplace_back(Ids::SaveGame, getSlotPos(0), [this](){}, false);
            _buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [this](){});
            _buttons.emplace_back(Ids::Sound, getSlotPos(2), [this](){ updateState(State::Sound); });
            _buttons.emplace_back(Ids::Video, getSlotPos(3), [this](){ updateState(State::Video); });
            _buttons.emplace_back(Ids::Controls, getSlotPos(4), [this](){ updateState(State::Controls); });
            _buttons.emplace_back(Ids::TextAndSpeech, getSlotPos(5), [this](){ updateState(State::TextAndSpeech); });
            _buttons.emplace_back(Ids::Help, getSlotPos(6), [this](){ updateState(State::Help); });
            _buttons.emplace_back(Ids::Quit, getSlotPos(7), [](){});
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [](){});
            break;
        case State::Sound:
            setHeading(Ids::Sound);
            _sliders.emplace_back(getSlotPos(1));
            _sliders.emplace_back(getSlotPos(2));
            _sliders.emplace_back(getSlotPos(3));
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); });
            break;
        case State::Video:
            setHeading(Ids::Video);
            _checkboxes.emplace_back(Ids::Fullscreen, getSlotPos(1));
            // Ids::SafeArea
            _sliders.emplace_back(getSlotPos(2));
            _checkboxes.emplace_back(Ids::ToiletPaperOver, getSlotPos(4));
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); });
            break;
        case State::Controls:
            setHeading(Ids::Controls);
            _checkboxes.emplace_back(Ids::Controller, getSlotPos(1), false);
            _checkboxes.emplace_back(Ids::ScrollSyncCursor, getSlotPos(2), false, true);
            _checkboxes.emplace_back(Ids::InvertVerbColors, getSlotPos(4));
            _checkboxes.emplace_back(Ids::RetroFonts, getSlotPos(5));
            _checkboxes.emplace_back(Ids::RetroVerbs, getSlotPos(6));
            _checkboxes.emplace_back(Ids::ClassicSentence, getSlotPos(7));
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); });
            break;
        case State::TextAndSpeech:
            setHeading(Ids::TextAndSpeech);
            // Ids::TextSpeed
            _sliders.emplace_back(getSlotPos(1));
            _checkboxes.emplace_back(Ids::DisplayText, getSlotPos(2));
            _checkboxes.emplace_back(Ids::HearVoice, getSlotPos(3));
            _switchButtons.push_back(SwitchButton({Ids::EnglishText, Ids::FrenchText, Ids::ItalianText, Ids::GermanText, Ids::SpanishText}, getSlotPos(5)));
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); });
            break;
        case State::Help:
            setHeading(Ids::Help);
            _buttons.emplace_back(Ids::Introduction, getSlotPos(1), [this](){});
            _buttons.emplace_back(Ids::MouseTips, getSlotPos(2), [this](){});
            _buttons.emplace_back(Ids::ControllerTips, getSlotPos(3), [this](){});
            _buttons.emplace_back(Ids::ControllerMap, getSlotPos(4), [this](){});
            // _buttons.emplace_back(Ids::KeyboardMap, getSlotPos(5), [this](){});
            _buttons.emplace_back(Ids::Back, getSlotPos(9), [this](){ updateState(State::Main); });
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

        updateState(State::Main);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states)
    {
        const auto view = target.getView();
        auto viewRect = sf::FloatRect(0, 0, 320, 180);
        target.setView(sf::View(viewRect));
        
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
    }

    void update(const sf::Time& elapsed)
    {
        auto pos = _pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()));
        for(auto& button : _buttons) {
            button.update();
        }
        for(auto& switchButton : _switchButtons) {
            switchButton.update();
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
}
