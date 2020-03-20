#include "_Button.hpp"
#include "_Checkbox.hpp"
#include "_Slider.hpp"
#include "_SwitchButton.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Preferences.hpp"
#include "Font/FntFont.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/SpriteSheet.hpp"
#include "Graphics/Text.hpp"
#include "System/Logger.hpp"
#include "UI/OptionsDialog.hpp"
#include "UI/StartScreenDialog.hpp"
#include "UI/QuitDialog.hpp"
#include "imgui.h"


namespace ng
{
struct StartScreenDialog::Impl
{
    enum class State {Main, Options, Help, Quit};

    struct Ids {
        inline static const int LoadGame = 99910;
        inline static const int NewGame=99912;
        inline static const int Options=99913;
        inline static const int Quit=99915;
        inline static const int Help=99961;
    };   

    static constexpr float yPosStart = 84.f;
    static constexpr float yPosLarge = 58.f;
    static constexpr float yPosSmall = 54.f;

    Engine* _pEngine{nullptr};

    std::vector<_Button> _buttons;
    QuitDialog _quit;
    OptionsDialog _options;
    State _state{State::Main};
    Callback _newGameCallback;

    inline static float getSlotPos(int slot)
    {
        return yPosStart+yPosLarge+yPosSmall*slot;
    }

    void updateState(State state)
    {
        _state = state;
        _buttons.clear();
        switch(state)
        {
        case State::Main:
            _buttons.emplace_back(Ids::LoadGame, getSlotPos(1), [](){}, false);
            _buttons.emplace_back(Ids::NewGame, getSlotPos(2), [this](){ if(_newGameCallback) _newGameCallback(); });
            _buttons.emplace_back(Ids::Options, getSlotPos(3), [this](){ updateState(State::Options); });
            _buttons.emplace_back(Ids::Help, getSlotPos(4), [this](){ updateState(State::Help); });
            _buttons.emplace_back(Ids::Quit, getSlotPos(5), [this](){ updateState(State::Quit); });
            break;
        case State::Help:
            _options.showHelp();
            break;
        case State::Options:
            break;
        case State::Quit:
            break;
        default:
            updateState(State::Main);
            break;
        }

        for(auto& button : _buttons) {
            button.setEngine(_pEngine);
        }
    }

    void setEngine(Engine* pEngine)
    {
        _pEngine=pEngine;
        if(!pEngine) return;

        _options.setEngine(pEngine);
        _options.setCallback([this](){
            updateState(State::Main);
        });

        _quit.setEngine(pEngine);
        _quit.setCallback([this](bool result){
            if(result) _pEngine->quit();
            updateState(State::Main);
        });

        updateState(State::Main);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states)
    {
        switch(_state)
        {
        case State::Main:
        {
            const auto view = target.getView();
            auto viewRect = sf::FloatRect(0, 0, Screen::Width, Screen::Height);
            target.setView(sf::View(viewRect));

            // controls
            for(auto& button : _buttons) {
                target.draw(button);
            }
            
            target.setView(view);
            break;
        }

        case State::Options:
        case State::Help:
            target.draw(_options, states);
            break;

        case State::Quit:
            target.draw(_quit, states);
            break;
        }
    }

    void update(const sf::Time& elapsed)
    {
        switch(_state)
        {
        case State::Quit:
            _quit.update(elapsed);
            break;

        case State::Options:
        case State::Help:
            _options.update(elapsed);
            break;

        default:
            auto pos = (sf::Vector2f)_pEngine->getWindow().mapPixelToCoords(sf::Mouse::getPosition(_pEngine->getWindow()), sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));
            for(auto& button : _buttons) {
                button.update(pos);
            }
            break;
        }
    }
};

StartScreenDialog::StartScreenDialog()
: _pImpl(std::make_unique<Impl>())
{
}

StartScreenDialog::~StartScreenDialog() = default;

void StartScreenDialog::setEngine(Engine* pEngine) { _pImpl->setEngine(pEngine); }

void StartScreenDialog::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    _pImpl->draw(target, states);
}

void StartScreenDialog::update(const sf::Time& elapsed)
{
    _pImpl->update(elapsed);
}

void StartScreenDialog::setNewGameCallback(Callback callback)
{
    _pImpl->_newGameCallback = callback;
}
}
