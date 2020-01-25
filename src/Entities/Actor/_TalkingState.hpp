#pragma once
#include "SFML/Graphics.hpp"
#include "Engine/Engine.hpp"
#include "Font/Font.hpp"
#include "Graphics/Screen.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "_LipAnimation.hpp"
#include "../../System/_Util.hpp"

namespace ng
{
class _TalkingState : public sf::Drawable, public sf::Transformable
{
public:
    _TalkingState() = default;

    void setEngine(Engine* pEngine)
    { 
        _pEngine = pEngine;
    }
    
    void update(const sf::Time &elapsed)
    {
        if (!_isTalking)
            return;

        bool end = false;
        _elapsed += elapsed;
        auto pSound = dynamic_cast<SoundId*>(ScriptEngine::getSoundFromId(_soundId));
        if(pSound)
        {
            end = !pSound->isPlaying();
        }
        else {
            end = _elapsed > _duration;
        }

        if(end) {
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
        trace("Talk duration: {}", _duration.asSeconds());
        _elapsed = sf::seconds(0);
    }
    
    inline void setText(const std::wstring& text) { _sayText = text; }
    
    void loadLip(const std::string& text, Actor* pActor)
    { 
        _pActor = pActor;
        setTalkColor(pActor->getTalkColor());

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
        setText(_pEngine->getText(id));

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

        setDuration(_lipAnim.getDuration());
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        if (!_isTalking)
            return;

        auto screen = target.getView().getSize();
        auto scale = screen.y / (2.f * 512.f);

        auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
        const Font& font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet": "FontModernSheet");

        NGText text;
        text.scale(scale, scale);
        text.setAlignment(NGTextAlignment::Center);
        text.setFont(font);
        text.setColor(_talkColor);
        text.setText(_sayText);

        states.transform *= getTransform();

        target.draw(text, states);
    }

private:
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