#include "_ControlConstants.hpp"
#include "Engine/Engine.hpp"
#include "Font/FntFont.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Graphics/Text.hpp"
#include "UI/QuitDialog.hpp"
#include "imgui.h"

namespace ng
{
class _QuitButton: public sf::Drawable
{
public:
    typedef std::function<void()> Callback;

public:
    _QuitButton(int id, bool value, Callback callback, bool enabled = true)
    : _id(id), _isEnabled(enabled), _value(value), _callback(std::move(callback))
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontLarge = _pEngine->getTextureManager().getFntFont("UIFontLarge.fnt");
        text.setFont(uiFontLarge);
        text.setString(_pEngine->getText(_id));
        auto textRect = text.getLocalBounds();
        auto originX = _value?textRect.width:0;
        auto x = _value?-40.f:40.f;
        text.setOrigin(sf::Vector2f(originX, textRect.height/2.f));
        text.setPosition(sf::Vector2f(Screen::Width/2.0f+x, 400.f));
    }

    void update(sf::Vector2f pos)
    {
        auto textRect = text.getGlobalBounds();
        sf::Color color;
        if(!_isEnabled)
        {
            color=_ControlConstants::DisabledColor;
        }
        else if(textRect.contains((sf::Vector2f)pos))
        {
            color=_ControlConstants::HoveColor;
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
            color=_ControlConstants::NormalColor;
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
    bool _value{false};
    bool _wasMouseDown{false};
    Callback _callback;
    Text text;
};

struct QuitDialog::Impl
{
    struct Ids {
        static constexpr int Yes      = 99907;
        static constexpr int No       = 99908;
        static constexpr int QuitText = 99909;
    };

    Engine* _pEngine{nullptr};
    SpriteSheet _saveLoadSheet;
    Text _headingText;
    std::vector<_QuitButton> _buttons;
    Callback _callback{nullptr};

    void setHeading(int id)
    {
        _headingText.setString(_pEngine->getText(id));
        auto textRect = _headingText.getGlobalBounds();
        _headingText.setPosition(sf::Vector2f((Screen::Width-textRect.width)/2.f, 240.f));
    }

    void updateState()
    {
        _buttons.clear();
        
        setHeading(Ids::QuitText);

        _buttons.emplace_back(Ids::Yes, true, [this](){ if(_callback) _callback(true); });
        _buttons.emplace_back(Ids::No, false, [this](){ if(_callback) _callback(false); });
            
        for(auto& button : _buttons) {
            button.setEngine(_pEngine);
        }
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;
        if(!pEngine) return;

        TextureManager& tm = pEngine->getTextureManager();
        _saveLoadSheet.setTextureManager(&tm);
        _saveLoadSheet.load("SaveLoadSheet");

        const FntFont& headingFont = _pEngine->getTextureManager().getFntFont("UIFontMedium.fnt");
        _headingText.setFont(headingFont);
        _headingText.setFillColor(sf::Color::White);

        updateState();
    }

    void draw(sf::RenderTarget& target, sf::RenderStates)
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
        auto rect = _saveLoadSheet.getRect("error_dialog_small");
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
        target.setView(view);
    }

    void update(const sf::Time&)
    {
        auto pos = (sf::Vector2f)_pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()), sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));
        for(auto& button : _buttons) {
            button.update(pos);
        }
    }
};

QuitDialog::QuitDialog()
: _pImpl(std::make_unique<Impl>())
{
}

QuitDialog::~QuitDialog() = default;

void QuitDialog::setEngine(Engine* pEngine) { _pImpl->setEngine(pEngine); }

void QuitDialog::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    _pImpl->draw(target, states);
}

void QuitDialog::update(const sf::Time& elapsed)
{
    _pImpl->update(elapsed);
}

void QuitDialog::setCallback(Callback callback)
{
    _pImpl->_callback = callback;
}
}
