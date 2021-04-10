#include <engge/Engine/EngineSettings.hpp>
#include <engge/Graphics/Text.hpp>
#include "TalkingState.hpp"

namespace ng {
void TalkingState::setPosition(glm::vec2 pos) {
  m_transform.setPosition(pos);
}

void TalkingState::setEngine(Engine *pEngine) {
  m_pEngine = pEngine;
}

void TalkingState::update(const ngf::TimeSpan &elapsed) {
  if (!m_isTalking)
    return;

  bool end;
  m_elapsed += elapsed;
  auto pSound = dynamic_cast<SoundId *>(EntityManager::getSoundFromId(m_soundId));
  if (pSound) {
    end = !pSound->isPlaying();
  } else {
    end = m_elapsed > m_duration;
  }

  if (end) {
    if (m_ids.empty()) {
      m_isTalking = false;
      m_lipAnim.end();
      return;
    }
    auto[id, text, mumble] = m_ids.front();
    loadId(id, text, mumble);
    m_ids.erase(m_ids.begin());
  }
  m_lipAnim.update(elapsed);
}

void TalkingState::stop() {
  m_ids.clear();
  m_isTalking = false;
  if (m_soundId) {
    auto pSound = dynamic_cast<SoundId *>(EntityManager::getSoundFromId(m_soundId));
    if (pSound) {
      pSound->stop();
    }
    m_soundId = 0;
  }
}

bool TalkingState::isTalking() const { return m_isTalking; }

void TalkingState::setTalkColor(ngf::Color color) { m_talkColor = color; }

void TalkingState::setDuration(ngf::TimeSpan duration) {
  m_isTalking = true;
  m_duration = duration;
  trace("Talk duration: {}", m_duration.getTotalSeconds());
  m_elapsed = ngf::TimeSpan::seconds(0);
}

void TalkingState::setText(const std::wstring &text) { m_sayText = text; }

void TalkingState::loadLip(const std::string &text, Entity *pEntity, bool mumble) {
  m_pEntity = pEntity;
  setTalkColor(pEntity->getTalkColor());

  // load lip data
  auto id = std::strtol(text.c_str() + 1, nullptr, 10);

  if (m_isTalking) {
    m_ids.emplace_back(id, text, mumble);
    return;
  }

  loadId(id, text, mumble);
}

void TalkingState::draw(ngf::RenderTarget &target, ngf::RenderStates) const {
  if (!m_isTalking)
    return;

  if (!m_pEngine->getPreferences().getUserPreference(PreferenceNames::DisplayText, PreferenceDefaultValues::DisplayText))
    return;

  auto view = target.getView();
  target.setView(ngf::View(ngf::frect::fromPositionSize({0, 0}, {Screen::Width, Screen::Height})));

  auto retroFonts = m_pEngine->getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                  PreferenceDefaultValues::RetroFonts);
  auto &font = m_pEngine->getResourceManager().getFont(retroFonts ? "FontRetroSheet" : "FontModernSheet");

  ng::Text text;
  text.setMaxWidth(static_cast<int>((Screen::Width * 3) / 4));
  text.setFont(font);
  text.setColor(m_talkColor);
  text.setWideString(m_sayText);

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

  auto soundDefinition = m_pEngine->getSoundManager().defineSound(name + ".ogg");
  if (!soundDefinition) {
    error("File {}.ogg not found", name);
    return;
  }

  auto pSound = m_pEngine->getSoundManager().playTalkSound(soundDefinition, 1, ngf::TimeSpan::Zero, m_pEntity->getId());
  if (pSound) {
    m_soundId = pSound->getId();
  }
}

void TalkingState::loadId(int id, const std::string &text, bool mumble) {
  ScriptEngine::callFunc(id, "onTalkieID", m_pEntity, id);
  auto sayText = id != 0 ? Engine::getText(id) : towstring(text);
  setText(sayText);

  const char *key = nullptr;
  if (!ScriptEngine::rawGet(m_pEntity, "_talkieKey", key)) {
    ScriptEngine::rawGet(m_pEntity, "_key", key);
  }
  auto name = str_toupper(key).append("_").append(std::to_string(id));
  std::string path;
  path.append(name).append(".lip");

  auto pActor = dynamic_cast<Actor *>(m_pEntity);
  if (pActor) {
    m_lipAnim.setActor(pActor);
  }

  // actor animation
  std::wregex re(LR"(\{([^\}]*)\})");
  std::wsmatch matches;
  std::string anim;

  if (std::regex_search(m_sayText, matches, re)) {
    anim = tostring(matches[1].str());
    m_sayText = matches.suffix();
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
    m_lipAnim.load(path);
  } else {
    m_lipAnim.clear();
  }

  auto hearVoice = (id != 0) && (m_pEngine->getPreferences().getTempPreference(TempPreferenceNames::ForceTalkieText,
                                                                               TempPreferenceDefaultValues::ForceTalkieText)
      || m_pEngine->getPreferences().getUserPreference(PreferenceNames::HearVoice,
                                                       PreferenceDefaultValues::HearVoice));
  if (hearVoice) {
    setDuration(m_lipAnim.getDuration());
  } else {
    auto sayLineBaseTime = m_pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineBaseTime,
                                                                         PreferenceDefaultValues::SayLineBaseTime);
    auto sayLineCharTime = m_pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineCharTime,
                                                                         PreferenceDefaultValues::SayLineCharTime);
    auto sayLineMinTime = m_pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineMinTime,
                                                                        PreferenceDefaultValues::SayLineMinTime);
    auto sayLineSpeed = m_pEngine->getPreferences().getUserPreference(PreferenceNames::SayLineSpeed,
                                                                      PreferenceDefaultValues::SayLineSpeed);
    auto speed = (sayLineBaseTime + sayLineCharTime * m_sayText.length()) / (0.2f + sayLineSpeed);
    if (speed < sayLineMinTime)
      speed = sayLineMinTime;
    setDuration(ngf::TimeSpan::seconds(speed));
  }

  auto sayLine = tostring(m_sayText);
  const char *pAnim = anim.empty() ? nullptr : anim.data();
  ScriptEngine::rawCall(m_pEntity, "sayingLine", pAnim, sayLine);

  loadActorSpeech(name, hearVoice);
}
}
