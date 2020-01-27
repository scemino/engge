#pragma once
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "_ControlConstants.hpp"
#include "Engine/Engine.hpp"
#include "Font/FntFont.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/Text.hpp"

namespace ng
{
class _Button: public sf::Drawable
{
public:
    typedef std::function<void()> Callback;
    enum class Size {Large, Medium};

public:
    _Button(int id, float y, Callback callback, bool enabled = true, Size size = Size::Large)
    : _id(id), _isEnabled(enabled), _y(y), _callback(std::move(callback)), _size(size)
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontLargeOrMedium = _pEngine->getTextureManager().getFntFont(_size == Size::Large ? "UIFontLarge.fnt":"UIFontMedium.fnt");
        text.setFont(uiFontLargeOrMedium);
        text.setString(_pEngine->getText(_id));
        auto textRect = text.getLocalBounds();
        text.setOrigin(sf::Vector2f(textRect.width/2.f, 0));
        text.setPosition(sf::Vector2f(Screen::Width/2.f, _y));
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
    float _y{0};
    bool _wasMouseDown{false};
    Callback _callback;
    Text text;
    Size _size{Size::Large};
};
}
