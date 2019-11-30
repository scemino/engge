#pragma once
#include "SFML/Graphics.hpp"
#include "Engine.h"
#include "Font.h"
#include "Screen.h"
#include "ScriptEngine.h"
#include "_LipAnimation.h"
#include "_Util.h"

namespace ng
{
class _TalkingState : public sf::Drawable, public sf::Transformable
{
public:
    _TalkingState() = default;

    void setEngine(Engine* pEngine)
    { 
        _pEngine = pEngine;
        _lipAnim.setSettings(pEngine->getSettings());

        _font.setTextureManager(&_pEngine->getTextureManager());
        _font.setSettings(&_pEngine->getSettings());
        auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
        _font.load(retroFonts ? "FontRetroSheet": "FontModernSheet");
    }
    
    void update(const sf::Time &elapsed)
    {
        if (!_isTalking)
            return;

        _elapsed += elapsed;
        if (_elapsed > _duration)
        {
            if (_ids.empty())
            {
                _isTalking = false;
                return;
            }
            loadId(_ids.front());
            _ids.erase(_ids.begin());
        }
        _lipAnim.update(elapsed);
    }

    void stop()
    {
        _ids.clear();
        _isTalking = false;
        if (_soundId)
        {
            auto pSound = dynamic_cast<SoundId*>(ScriptEngine::getSoundFromId(_soundId));
            if(pSound)
            {
                pSound->stop();
            }
            _soundId = 0;
        }
    }
    
    inline bool isTalking() const { return _isTalking; }
    
    inline void setTalkColor(sf::Color color) { _talkColor = color; }
    inline sf::Color getTalkColor() const { return _talkColor; }

    void setDuration(sf::Time duration)
    { 
        _isTalking = true; 
        _duration = duration;
        _elapsed = sf::seconds(0);
    }
    
    inline void setText(const std::wstring& text) { _sayText = text; }
    
    void loadLip(const std::string& text, Actor* pActor)
    { 
        _pActor = pActor;
        setTalkColor(pActor->getTalkColor());
        setText(_pEngine->getText(text));

        // load lip data
        auto id = std::strtol(text.c_str()+1,nullptr,10);

        if (_isTalking)
        {
            _ids.push_back(id);
            return;
        }

        loadId(id);
    }

private:
    void loadId(int id)
    {
        std::string path;
        const char* key = nullptr;
        if(!ScriptEngine::get(_pActor, "_talkieKey", key))
        {
            ScriptEngine::get(_pActor, "_key", key);
        }
        std::string name = str_toupper(key).append("_").append(std::to_string(id));
        path.append(name).append(".lip");

        _lipAnim.setActor(_pActor);
        _lipAnim.load(path);

        // actor animation
        std::wregex re(L"(\\{([^\\}]*)\\})");
        std::wsmatch matches;
        const char* anim = nullptr;
        if (std::regex_search(_sayText, matches, re))
        {
            anim = tostring(matches[2].str()).data();
            _pActor->getCostume().setState(anim);
            _sayText = matches.suffix();
        }
        const char* sayLine = tostring(_sayText).data();
        ScriptEngine::call(_pActor, "sayingLine", anim, sayLine);

        // load actor voice
        auto soundDefinition = _pEngine->getSoundManager().defineSound(name + ".ogg");
        if (!soundDefinition)
        {
            error("File {}.ogg not found", name);
        }
        else
        {
            auto pSound = _pEngine->getSoundManager().playTalkSound(soundDefinition, 1, _pActor);
            if(pSound)
            {
                _soundId = pSound->getId();
            }
        }

        _duration = _lipAnim.getDuration();
        _isTalking = true;
        _elapsed = sf::seconds(0);
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        if (!_isTalking)
            return;

        auto screen = target.getView().getSize();
        auto scale = screen.y / (2.f * 512.f);
        NGText text;
        text.scale(scale, scale);
        text.setAlignment(NGTextAlignment::Center);
        text.setFont(_font);
        text.setColor(_talkColor);
        text.setText(_sayText);

        states.transform *= getTransform();

        target.draw(text, states);
    }

private:
    Font _font;
    Engine* _pEngine{nullptr};
    Actor* _pActor{nullptr};
    bool _isTalking{false};
    std::wstring _sayText;
    sf::Color _talkColor{sf::Color::White};
    sf::Time _elapsed;
    sf::Time _duration;
    _LipAnimation _lipAnim;
    int _soundId{0};
    std::vector<int> _ids;
};
}