#pragma once
#include "SFML/Graphics.hpp"
#include "Engine/Engine.hpp"
#include "Font/GGFont.hpp"
#include "Graphics/Screen.hpp"
#include "Graphics/Text.hpp"
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
    void loadActorSpeech(const std::string& name) {
        if(!_pEngine->getPreferences().getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice))
            return;

        auto soundDefinition = _pEngine->getSoundManager().defineSound(name + ".ogg");
        if (!soundDefinition)
        {
            error("File {}.ogg not found", name);
            return;
        }

        auto pSound = _pEngine->getSoundManager().playTalkSound(soundDefinition, 1, _pActor);
        if(pSound)
        {
            _soundId = pSound->getId();
        }
    }

    void loadId(int id)
    {
        setText(_pEngine->getText(id));

        const char* key = nullptr;
        if(!ScriptEngine::rawGet(_pActor, "_talkieKey", key))
        {
            ScriptEngine::rawGet(_pActor, "_key", key);
        }
        auto name = str_toupper(key).append("_").append(std::to_string(id));
        std::string path;
        path.append(name).append(".lip");

        _lipAnim.setActor(_pActor);

        // actor animation
        std::wregex re(LR"(\{([^\}]*)\})");
        std::wsmatch matches;
        std::string anim;

        bool loadLipAnim = true;
        if (std::regex_search(_sayText, matches, re))
        {
            anim = tostring(matches[1].str());
            _sayText = matches.suffix();
            if(anim == "notalk")
            {
                loadLipAnim = false;
            }
            else
            {
                _pActor->getCostume().setState(anim);
            }
        }

        if(loadLipAnim)
        {
            _lipAnim.load(path);
        }
        else 
        {
            _lipAnim.clear();
        }

        auto hearVoice = _pEngine->getPreferences().getUserPreference(PreferenceNames::HearVoice, PreferenceDefaultValues::HearVoice);
        if(hearVoice) {
            setDuration(_lipAnim.getDuration());
        } else {
            auto sayLineBaseTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineBaseTime, PreferenceDefaultValues::SayLineBaseTime);
            auto sayLineCharTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineCharTime, PreferenceDefaultValues::SayLineCharTime);
            auto sayLineMinTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineMinTime, PreferenceDefaultValues::SayLineMinTime);
            auto sayLineSpeed = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineSpeed, PreferenceDefaultValues::SayLineSpeed);
            auto speed = (sayLineBaseTime + sayLineCharTime * _sayText.length()) / (0.2f + sayLineSpeed);
            if(speed < sayLineMinTime) speed = sayLineMinTime;
            setDuration(sf::seconds(speed));
        }
        
        auto sayLine = tostring(_sayText);
        const char* pAnim = anim.empty()? nullptr : anim.data();
        ScriptEngine::rawCall(_pActor, "sayingLine", pAnim, sayLine);

        loadActorSpeech(name);
    }

    void draw(sf::RenderTarget &target, sf::RenderStates) const override
    {
        if (!_isTalking)
            return;

        if(!_pEngine->getPreferences().getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText))
            return;

        auto view = target.getView();
        target.setView(sf::View(sf::FloatRect(0,0,Screen::Width,Screen::Height)));

        auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts, PreferenceDefaultValues::RetroFonts);
        const GGFont& font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet": "FontModernSheet");

        Text text;
        text.setMaxWidth(static_cast<int>((Screen::Width*3)/4));
        text.setFont(font);
        text.setFillColor(_talkColor);
        text.setString(_sayText);

        auto bounds = text.getLocalBounds();
        auto pos = getPosition();

        if((pos.x + bounds.width/2) > (Screen::Width-20))
        {
            pos.x = Screen::Width-bounds.width - 20;
        }
        else if((pos.x - bounds.width/2) < 20)
        {
            pos.x = 20;
        }
        else
        {
            pos.x = pos.x-bounds.width / 2;
        }
        if((pos.y + bounds.height) > (Screen::Height-20))
        {
            pos.y = Screen::Height - 20 - bounds.height;
        }
        else if((pos.y - bounds.height) < 20)
        {
            pos.y = 20 + bounds.height;
        }
        else
        {
            pos.y = pos.y-bounds.height;
        }
        text.setPosition(pos);
        target.draw(text);

        // sf::RectangleShape shape;
        // shape.setFillColor(sf::Color::Transparent);
        // shape.setOutlineColor(sf::Color::White);
        // shape.setOutlineThickness(1.f);
        // shape.setSize(sf::Vector2f(bounds.width, bounds.height));
        // shape.setPosition(pos);
        // target.draw(shape);

        target.setView(view);
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