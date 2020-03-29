#pragma once
#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "_ControlConstants.hpp"

namespace ng
{
class _SwitchButton: public sf::Drawable
{
public:
    typedef std::function<void(int)> Callback;

public:
    _SwitchButton(std::initializer_list<int> ids, float y, bool enabled = true, int index = 0, Callback callback = nullptr)
    : _ids(ids), _index(index), _isEnabled(enabled), _y(y), _callback(std::move(callback))
    {
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;

        const FntFont& uiFontMedium = _pEngine->getTextureManager().getFntFont("UIFontMedium.fnt");
        text.setFont(uiFontMedium);
        text.setString(_pEngine->getText(_ids[_index]));
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
                _index=(_index+1)% static_cast<int>(_ids.size());
                text.setString(_pEngine->getText(_ids[_index]));
                if(_callback) {
                    _callback(_index);
                }
                textRect = text.getLocalBounds();
                text.setOrigin(sf::Vector2f(textRect.width/2.f, 0));
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
    std::vector<int> _ids;
    int _index{0};
    bool _isEnabled{true};
    float _y{0};
    bool _wasMouseDown{false};
    Callback _callback;
    Text text;
};
}
