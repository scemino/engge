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
    typedef std::function<void()>		Callback;

public:
    Button(int id, float y, Callback callback, bool enabled = true)
    : _id(id), _y(y), _callback(std::move(callback)), _isEnabled(enabled)
    {
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
    Checkbox(int id, float y, bool enabled = true)
    : _id(id), _y(y), _isEnabled(enabled)
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
    Engine* _pEngine{nullptr};
    SpriteSheet _saveLoadSheet;
    std::vector<Button> _buttons;
    std::vector<SwitchButton> _switchButtons;
    std::vector<Checkbox> _checkboxes;
    std::vector<Slider> _sliders;

    Impl()
    {
        const float yPosLarge = 58.f;
        const float yPosSmall = 54.f;
        // _buttons.emplace_back(99911,44.f+yPosLarge, [](){}, false);
        _sliders.emplace_back(44.f+yPosLarge+20.f);
        _sliders[0].setCallback([](float value){
            trace("value: {}", value);
        });
        _buttons.emplace_back(99910,44.f+yPosLarge+yPosSmall,   [](){}, false);
        _switchButtons.push_back(SwitchButton({98001,98003,98005,98007,98009},44.f+yPosLarge+yPosSmall*2));
        // _buttons.emplace_back(99916,44.f+yPosLarge+yPosSmall*2, [](){});
        _buttons.emplace_back(99917,44.f+yPosLarge+yPosSmall*3, [](){});
        _buttons.emplace_back(99918,44.f+yPosLarge+yPosSmall*4, [](){});
        _buttons.emplace_back(99919,44.f+yPosLarge+yPosSmall*5, [](){});
        // _buttons.emplace_back(99961,44.f+yPosLarge+yPosSmall*6, [](){});
        // _buttons.emplace_back(99915,44.f+yPosLarge+yPosSmall*7, [](){});
        _checkboxes.emplace_back(99965, 44.f+yPosLarge+yPosSmall*8);
        _checkboxes.emplace_back(99961, 44.f+yPosLarge+yPosSmall*9);
        _checkboxes[0].setCallback([this](bool value){
            if(!value) return;
            _checkboxes[1].setChecked(value);
        });
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;
        TextureManager& tm = pEngine->getTextureManager();
        _saveLoadSheet.setTextureManager(&tm);
        _saveLoadSheet.load("SaveLoadSheet");

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

    void draw(sf::RenderTarget& target, sf::RenderStates states)
    {
        auto rect = _saveLoadSheet.getRect("options_background");

        const auto view = target.getView();
        auto viewRect = sf::FloatRect(0, 0, 320, 180);
        
        target.setView(sf::View(viewRect));

        auto viewCenter = sf::Vector2f(viewRect.width/2,viewRect.height/2);
        
        // draw background
        sf::Sprite sprite;
        sprite.setPosition(viewCenter);
        sprite.setTexture(_saveLoadSheet.getTexture());
        sprite.setOrigin(rect.width/2,rect.height/2);
        sprite.setTextureRect(rect);
        target.draw(sprite);

        viewRect = sf::FloatRect(0, 0, Screen::Width, Screen::Height);
        target.setView(sf::View(viewRect));

        const FntFont& headingFont = _pEngine->getTextureManager().getFntFont("HeadingFont.fnt");
        const FntFont& uiFontLarge = _pEngine->getTextureManager().getFntFont("UIFontLarge.fnt");
        const FntFont& uiFontMedium = _pEngine->getTextureManager().getFntFont("UIFontMedium.fnt");

        const float yPosLarge = 88.f;
        auto yPos = 44.f;
        auto yPosBack = 594.f;
        
        Text text;
        text.setFont(headingFont);
        text.setFillColor(sf::Color::White);
        text.setString(_pEngine->getText(99913));
        auto textRect = text.getGlobalBounds();
        text.setPosition(sf::Vector2f((viewRect.width-textRect.width)/2.f, yPos-textRect.height/2));
        target.draw(text);

        yPos+=yPosLarge;

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
        
        // back
        text.setFont(uiFontMedium);
        text.setFillColor(sf::Color::White);
        text.setString(_pEngine->getText(99904));
        textRect = text.getGlobalBounds();
        text.setPosition(sf::Vector2f((viewRect.width-textRect.width)/2.f, yPosBack));
        target.draw(text);
        
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
