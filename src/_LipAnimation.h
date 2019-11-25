#pragma once
#include "SFML/Graphics.hpp"
#include "Actor.h"
#include "EngineSettings.h"
#include "Lip.h"

namespace ng
{
class _LipAnimation
{
public:
    void load(const std::string& path)
    {
        _lip.load(path);
        _index = 0;
        _elapsed = sf::seconds(0);
        updateHead();
    }

    void setSettings(EngineSettings &settings)
    {
        _lip.setSettings(settings);
    }

    inline void setActor(Actor* pActor)
    {
        _pActor = pActor;
    }

    void update(const sf::Time& elapsed)
    {
        if (_lip.getData().size() < 2) return;
        auto time = _lip.getData()[_index + 1].time;
        _elapsed += elapsed;
        if (_elapsed > time)
        {
            _index++;
        }
        if (_index >= _lip.getData().size())
        {
            _pActor->getCostume().setHeadIndex(0);
            return;
        }
        updateHead();
    }

    void updateHead()
    {
        if (_lip.getData().empty()) return;
        auto letter = _lip.getData()[_index].letter;
        if (letter == 'X' || letter == 'G')
            letter = 'A';
        if (letter == 'H')
            letter = 'D';
        auto index = letter - 'A';
        // TODO: what is the correspondance between letter and head index ?
        _pActor->getCostume().setHeadIndex(index);
    }

    sf::Time getDuration() const 
    {
        if(_lip.getData().empty()) return sf::seconds(0);
        return _lip.getData().back().time;
    }

private:
    Lip _lip;
    Actor* _pActor{nullptr};
    int _index{0};
    sf::Time _elapsed;
};
}
