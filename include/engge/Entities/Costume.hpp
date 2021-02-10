#pragma once
#include <optional>
#include <sstream>
#include <set>
#include <unordered_map>
#include "BlinkState.hpp"
#include "DirectionConstants.hpp"
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Graphics/ResourceManager.hpp>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/AnimControl.hpp>
#include <engge/Entities/Facing.hpp>

namespace ng {

enum class Reaching {
  High,
  Medium,
  Low
};

class Actor;

class Costume final : public ngf::Drawable {
public:
  explicit Costume(ResourceManager &textureManager);
  ~Costume() final;

  void loadCostume(const std::string &name, const std::string &sheet = "");
  [[nodiscard]] std::string getPath() const { return m_path; }
  [[nodiscard]] std::string getSheet() const { return m_sheet; }
  void lockFacing(Facing left, Facing right, Facing front, Facing back);
  void unlockFacing();
  void resetLockFacing();
  [[nodiscard]] std::optional<Facing> getLockFacing() const;

  void setFacing(Facing facing);
  [[nodiscard]] Facing getFacing() const;
  void setState(const std::string &name, bool loop = false);
  void setStandState() { setState(m_standAnimName); }
  void setWalkState() { setState(m_walkAnimName, true); }
  void setReachState(Reaching reaching);
  bool setAnimation(const std::string &name);
  Animation *getAnimation() { return m_pCurrentAnimation; }
  AnimControl& getAnimControl() { return m_animControl; }
  std::vector<Animation> &getAnimations() { return m_animations; }
  void setLayerVisible(const std::string &name, bool isVisible);
  void setHeadIndex(int index);
  [[nodiscard]] int getHeadIndex() const;

  void setAnimationNames(const std::string &headAnim,
                         const std::string &standAnim,
                         const std::string &walkAnim,
                         const std::string &reachAnim);
  void getAnimationNames(std::string &headAnim,
                         std::string &standAnim,
                         std::string &walkAnim,
                         std::string &reachAnim) const;

  void setActor(Actor *pActor) { m_pActor = pActor; }

  void setBlinkRate(float min, float max);

  void update(const ngf::TimeSpan &elapsed);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const final;

private:
  bool setMatchingAnimation(const std::string &animName);
  void updateAnimation();

private:
  ResourceManager &m_textureManager;
  std::string m_path;
  std::string m_sheet;
  std::vector<Animation> m_animations;
  Animation *m_pCurrentAnimation{nullptr};
  Facing m_facing{Facing::FACE_FRONT};
  std::set<std::string> m_hiddenLayers;
  std::string m_animation{"stand"};
  std::string m_standAnimName{"stand"};
  std::string m_headAnimName{"head"};
  std::string m_walkAnimName{"walk"};
  std::string m_reachAnimName{"reach"};
  int m_headIndex{0};
  Actor *m_pActor{nullptr};
  BlinkState m_blinkState;
  std::unordered_map<Facing, Facing> m_facings;
  bool m_lockFacing{false};
  SpriteSheet m_costumeSheet;
  AnimControl m_animControl;
};
} // namespace ng