#pragma once
#include "SFML/Graphics.hpp"
#include "engge/Engine/Engine.hpp"
#include "engge/Engine/EntityManager.hpp"
#include "engge/Graphics/GGFont.hpp"
#include "engge/Graphics/Screen.hpp"
#include "engge/Graphics/Text.hpp"
#include "engge/Scripting/ScriptEngine.hpp"
#include "_LipAnimation.hpp"
#include "../../System/_Util.hpp"
#include "engge/Audio/SoundId.hpp"
#include "engge/Engine/Preferences.hpp"
#include "engge/Audio/SoundManager.hpp"

namespace ng {
class _TalkingState : public sf::Drawable, public sf::Transformable {
public:
  _TalkingState() = default;

  void setEngine(Engine *pEngine) {
    _pEngine = pEngine;
  }

  void update(const sf::Time &elapsed) {
    if (!_isTalking)
      return;

    bool end;
    _elapsed += elapsed;
    auto pSound = dynamic_cast<SoundId *>(EntityManager::getSoundFromId(_soundId));
    if (pSound) {
      end = !pSound->isPlaying();
    } else {
      end = _elapsed > _duration;
    }

    if (end) {
      if (_ids.empty()) {
        _isTalking = false;
        _lipAnim.end();
        return;
      }
      auto[id, text, mumble] = _ids.front();
      loadId(id, text, mumble);
      _ids.erase(_ids.begin());
    }
    _lipAnim.update(elapsed);
  }

  void stop() {
    _ids.clear();
    _isTalking = false;
    if (_soundId) {
      auto pSound = dynamic_cast<SoundId *>(EntityManager::getSoundFromId(_soundId));
      if (pSound) {
        pSound->stop();
      }
      _soundId = 0;
    }
  }

  inline bool isTalking() const { return _isTalking; }

  inline void setTalkColor(sf::Color color) { _talkColor = color; }

  void setDuration(sf::Time duration) {
    _isTalking = true;
    _duration = duration;
    trace("Talk duration: {}", _duration.asSeconds());
    _elapsed = sf::seconds(0);
  }

  inline void setText(const std::wstring &text) { _sayText = text; }

  void loadLip(const std::string &text, Entity *pEntity, bool mumble = false) {
    _pEntity = pEntity;
    setTalkColor(pEntity->getTalkColor());

    // load lip data
    auto id = std::strtol(text.c_str() + 1, nullptr, 10);

    if (_isTalking) {
      _ids.emplace_back(id, text, mumble);
      return;
    }

    loadId(id, text, mumble);
  }

private:
  void loadActorSpeech(const std::string &name, bool hearVoice) {
    if (!hearVoice)
      return;

    auto soundDefinition = _pEngine->getSoundManager().defineSound(name + ".ogg");
    if (!soundDefinition) {
      error("File {}.ogg not found", name);
      return;
    }

    auto pSound = _pEngine->getSoundManager().playTalkSound(soundDefinition, 1, _pEntity->getId());
    if (pSound) {
      _soundId = pSound->getId();
    }
  }

  void loadId(int id, const std::string &text, bool mumble) {
    ScriptEngine::callFunc(id, "onTalkieID", _pEntity, id);
    auto sayText = id != 0 ? Engine::getText(id) : towstring(text);
    setText(sayText);

    const char *key = nullptr;
    if (!ScriptEngine::rawGet(_pEntity, "_talkieKey", key)) {
      ScriptEngine::rawGet(_pEntity, "_key", key);
    }
    auto name = str_toupper(key).append("_").append(std::to_string(id));
    std::string path;
    path.append(name).append(".lip");

    auto pActor = dynamic_cast<Actor *>(_pEntity);
    if (pActor) {
      _lipAnim.setActor(pActor);
    }

    // actor animation
    std::wregex re(LR"(\{([^\}]*)\})");
    std::wsmatch matches;
    std::string anim;

    bool loadLipAnim = !mumble;
    if (std::regex_search(_sayText, matches, re)) {
      anim = tostring(matches[1].str());
      _sayText = matches.suffix();
      if (!pActor || anim == "notalk") {
        loadLipAnim = false;
      } else {
        pActor->getCostume().setState(anim);
      }
    }

    if (pActor && loadLipAnim) {
      _lipAnim.load(path);
    } else {
      _lipAnim.clear();
    }

    auto hearVoice = (id != 0) && (_pEngine->getPreferences().getTempPreference(TempPreferenceNames::ForceTalkieText,
                                                                                TempPreferenceDefaultValues::ForceTalkieText)
        || _pEngine->getPreferences().getUserPreference(PreferenceNames::HearVoice,
                                                        PreferenceDefaultValues::HearVoice));
    if (hearVoice) {
      setDuration(_lipAnim.getDuration());
    } else {
      auto sayLineBaseTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineBaseTime,
                                                                          PreferenceDefaultValues::SayLineBaseTime);
      auto sayLineCharTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineCharTime,
                                                                          PreferenceDefaultValues::SayLineCharTime);
      auto sayLineMinTime = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineMinTime,
                                                                         PreferenceDefaultValues::SayLineMinTime);
      auto sayLineSpeed = _pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineSpeed,
                                                                       PreferenceDefaultValues::SayLineSpeed);
      auto speed = (sayLineBaseTime + sayLineCharTime * _sayText.length()) / (0.2f + sayLineSpeed);
      if (speed < sayLineMinTime)
        speed = sayLineMinTime;
      setDuration(sf::seconds(speed));
    }

    auto sayLine = tostring(_sayText);
    const char *pAnim = anim.empty() ? nullptr : anim.data();
    ScriptEngine::rawCall(_pEntity, "sayingLine", pAnim, sayLine);

    loadActorSpeech(name, hearVoice);
  }

  void draw(sf::RenderTarget &target, sf::RenderStates) const override {
    if (!_isTalking)
      return;

    if (!_pEngine->getPreferences().getUserPreference(PreferenceNames::DisplayText,
                                                      PreferenceDefaultValues::DisplayText))
      return;

    auto view = target.getView();
    target.setView(sf::View(sf::FloatRect(0, 0, Screen::Width, Screen::Height)));

    auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                   PreferenceDefaultValues::RetroFonts);
    const GGFont &font = _pEngine->getTextureManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

    Text text;
    text.setMaxWidth(static_cast<int>((Screen::Width * 3) / 4));
    text.setFont(font);
    text.setFillColor(_talkColor);
    text.setString(_sayText);

    auto bounds = text.getLocalBounds();
    auto pos = getPosition();

    if ((pos.x + bounds.width / 2) > (Screen::Width - 20)) {
      pos.x = Screen::Width - bounds.width - 20;
    } else if ((pos.x - bounds.width / 2) < 20) {
      pos.x = 20;
    } else {
      pos.x = pos.x - bounds.width / 2;
    }
    if ((pos.y + bounds.height) > (Screen::Height - 20)) {
      pos.y = Screen::Height - 20 - bounds.height;
    } else if ((pos.y - bounds.height) < 20) {
      pos.y = 20 + bounds.height;
    } else {
      pos.y = pos.y - bounds.height;
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
  Engine *_pEngine{nullptr};
  Entity *_pEntity{nullptr};
  bool _isTalking{false};
  std::wstring _sayText;
  sf::Color _talkColor{sf::Color::White};
  sf::Time _elapsed;
  sf::Time _duration;
  _LipAnimation _lipAnim;
  int _soundId{0};
  std::vector<std::tuple<int, std::string, bool>> _ids;
};
}