#pragma once
#include <algorithm>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/AnimState.hpp>

namespace ng {
class AnimControl {
public:
  void setAnimation(Animation *anim);
  Animation *getAnimation();

  void play(bool loop = false);
  void resume(bool loop = false);
  void stop();
  void pause();

  [[nodiscard]] AnimState getState() const;

  void update(const ngf::TimeSpan &e);
  [[nodiscard]] bool getLoop() const;

private:
  static void resetAnim(Animation &anim);
  static void rewind(Animation &anim);
  void update(const ngf::TimeSpan &e, Animation &animation) const;
  [[nodiscard]] static int getFps(const Animation &animation);

private:
  static void trig(const Animation &animation);

private:
  Animation *m_anim{nullptr};
  bool m_loop{false};
};
}