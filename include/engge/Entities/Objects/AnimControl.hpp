#pragma once
#include <algorithm>
#include <engge/Entities/Objects/ObjectAnimation.hpp>
#include <engge/Entities/Objects/AnimState.hpp>

namespace ng {
class AnimControl {
public:
  void setAnimation(ObjectAnimation *anim);
  ObjectAnimation *getAnimation();

  void play(bool loop = false);
  void stop();
  void pause();

  [[nodiscard]] AnimState getState() const;

  void update(const ngf::TimeSpan &e);
  [[nodiscard]] bool getLoop() const;

private:
  static void resetAnim(ObjectAnimation &anim);
  static void rewind(ObjectAnimation &anim);
  void update(const ngf::TimeSpan &e, ObjectAnimation &animation) const;
  [[nodiscard]] static int getFps(const ObjectAnimation &animation);

private:
  static void trig(const ObjectAnimation &animation);

private:
  ObjectAnimation *m_anim{nullptr};
  bool m_loop{false};
};
}