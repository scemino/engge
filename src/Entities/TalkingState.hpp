#pragma once
#include <ngf/Graphics/Text.h>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/EntityManager.hpp>
#include <engge/Graphics/GGFont.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Audio/SoundId.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Audio/SoundManager.hpp>
#include "LipAnimation.hpp"
#include "Util/Util.hpp"

namespace ng {
class TalkingState final : public ngf::Drawable {
public:
  TalkingState() = default;
  ~TalkingState() override = default;

  void setPosition(glm::vec2 pos);

  void setEngine(Engine *pEngine);

  void update(const ngf::TimeSpan &elapsed);

  void stop();

  bool isTalking() const;

  void setTalkColor(ngf::Color color);

  void setDuration(ngf::TimeSpan duration);

  void setText(const std::wstring &text);

  void loadLip(const std::string &text, Entity *pEntity, bool mumble = false);

  void draw(ngf::RenderTarget &target, ngf::RenderStates) const override;

private:
  void loadActorSpeech(const std::string &name, bool hearVoice);
  void loadId(int id, const std::string &text, bool mumble);

private:
  Engine *m_pEngine{nullptr};
  Entity *m_pEntity{nullptr};
  bool m_isTalking{false};
  std::wstring m_sayText;
  ngf::Color m_talkColor{ngf::Colors::White};
  ngf::TimeSpan m_elapsed;
  ngf::TimeSpan m_duration;
  LipAnimation m_lipAnim;
  int m_soundId{0};
  std::vector<std::tuple<int, std::string, bool>> m_ids;
  ngf::Transform m_transform;
};
}