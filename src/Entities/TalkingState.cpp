#include <engge/Engine/EngineSettings.hpp>
#include "TalkingState.hpp"

namespace ng {
void TalkingState::setPosition(glm::vec2 pos) {
  m_transform.setPosition(pos);
}

void TalkingState::setEngine(Engine *pEngine) {
  _pEngine = pEngine;
}

void TalkingState::update(const ngf::TimeSpan &elapsed) {
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

void TalkingState::stop() {
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

bool TalkingState::isTalking() const { return _isTalking; }

void TalkingState::setTalkColor(ngf::Color color) { _talkColor = color; }

void TalkingState::setDuration(ngf::TimeSpan duration) {
  _isTalking = true;
  _duration = duration;
  trace("Talk duration: {}", _duration.getTotalSeconds());
  _elapsed = ngf::TimeSpan::seconds(0);
}

void TalkingState::setText(const std::wstring &text) { _sayText = text; }

void TalkingState::loadLip(const std::string &text, Entity *pEntity, bool mumble) {
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

void TalkingState::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (!_isTalking)
    return;

  if (!_pEngine->getPreferences().getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText))
    return;

  auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts = _pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
  auto &font = _pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  Text text;
  text.setMaxWidth(static_cast<int>((Screen::Width * 3) / 4));
  text.setFont(font);
  text.setColor(_talkColor);
  text.setWideString(_sayText);

  auto bounds = text.getLocalBounds();
  auto pos = m_transform.getPosition();

  if ((pos.x + bounds.getWidth() / 2) > (Screen::Width - 20)) {
    pos.x = Screen::Width - bounds.getWidth() - 20;
  } else if ((pos.x - bounds.getWidth() / 2) < 20) {
    pos.x = 20;
  } else {
    pos.x = pos.x - bounds.getWidth() / 2;
  }
  if ((pos.y + bounds.getHeight()) > (Screen::Height - 20)) {
    pos.y = Screen::Height - 20 - bounds.getHeight();
  } else if ((pos.y - bounds.getHeight()) < 20) {
    pos.y = 20 + bounds.getHeight();
  } else {
    pos.y = pos.y - bounds.getHeight();
  }
  text.getTransform().setPosition(pos);
  text.draw(target, {});

// sf::RectangleShape shape;
// shape.setFillColor(sf::Color::Transparent);
// shape.setOutlineColor(sf::Color::White);
// shape.setOutlineThickness(1.f);
// shape.setSize(sf::Vector2f(bounds.width, bounds.height));
// shape.setPosition(pos);
// target.draw(shape);

  target.setView(view);
}

void TalkingState::loadActorSpeech(const std::string &name, bool hearVoice) {
  if (!hearVoice)
    return;

  auto soundDefinition = _pEngine->getSoundManager().defineSound(name + ".ogg");
  if (!soundDefinition) {
    error("File {}.ogg not found", name);
    return;
  }

  auto pSound = _pEngine->getSoundManager().playTalkSound(soundDefinition, 1, ngf::TimeSpan::Zero, _pEntity->getId());
  if (pSound) {
    _soundId = pSound->getId();
  }
}

void TalkingState::loadId(int id, const std::string &text, bool mumble) {
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

  if (std::regex_search(_sayText, matches, re)) {
    anim = tostring(matches[1].str());
    _sayText = matches.suffix();
    if (!pActor || anim == "notalk") {
      mumble = true;
    } else {
      pActor->getCostume().setState(anim);
    }
  }

  // force mumble if there is no lip file see issue #234
  if (!mumble) {
    mumble = !Locator<EngineSettings>::get().hasEntry(path);
  }

  if (pActor && !mumble) {
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
    setDuration(ngf::TimeSpan::seconds(speed));
  }

  auto sayLine = tostring(_sayText);
  const char *pAnim = anim.empty() ? nullptr : anim.data();
  ScriptEngine::rawCall(_pEntity, "sayingLine", pAnim, sayLine);

  loadActorSpeech(name, hearVoice);
}
}
