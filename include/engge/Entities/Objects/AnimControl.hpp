#pragma once
#include <engge/Entities/Objects/ObjectAnimation.hpp>

namespace ng {

enum class AnimState {
  Stopped,
  Play,
  Pause
};

class AnimControl {
public:
  void setAnimation(ObjectAnimation *anim) {
    m_anim = anim;
    stop();
  }
  ObjectAnimation *getAnimation() { return m_anim; }

  void play(bool loop = false) {
    m_loop = loop;
    if (!m_anim)
      return;
    m_state = AnimState::Play;
    rewind(*m_anim);
  }

  void stop() {
    if (!m_anim)
      return;
    m_state = AnimState::Stopped;
    resetAnim(*m_anim);
  }

  void pause() { m_state = AnimState::Pause; }

  [[nodiscard]] AnimState getState() const { return m_state; }

  void update(const ngf::TimeSpan &e) {
    if (m_state != AnimState::Play)
      return;
    if (!m_anim)
      return;

    if (m_anim->frames.empty() && m_anim->layers.empty())
      return;

    if (!m_anim->frames.empty()) {
      update(e, *m_anim, 1);
      return;
    }

    for (auto &layer : m_anim->layers) {
      update(e, layer, 2);
    }
  }

private:
  static void resetAnim(ObjectAnimation &anim) {
    if (!anim.frames.empty()) {
      anim.frameIndex = static_cast<int>(anim.frames.size()) - 1;
    }
    if (!anim.layers.empty()) {
      std::for_each(anim.layers.begin(), anim.layers.end(), resetAnim);
    }
  }

  static void rewind(ObjectAnimation &anim) {
    if (!anim.frames.empty()) {
      anim.frameIndex = 0;
    }
    if (!anim.layers.empty()) {
      std::for_each(anim.layers.begin(), anim.layers.end(), rewind);
    }
  }

  void update(const ngf::TimeSpan &e, ObjectAnimation &animation, int depth) {
    animation.elapsed += e;
    auto fps = getFps(animation);
    assert(fps > 0);

    // frame time elapsed?
    const auto frameTime = 1.f / static_cast<float>(fps);
    if (animation.elapsed.getTotalSeconds() <= frameTime)
      return;

    animation.elapsed = ngf::TimeSpan::seconds(animation.elapsed.getTotalSeconds() - frameTime);
    animation.frameIndex++;

    // quit if animation length not reached
    if (animation.frameIndex != static_cast<int>(animation.frames.size()))
      return;

    // loop if requested
    if (m_loop || animation.loop) {
      animation.frameIndex = 0;
      trig(animation);
      return;
    }

    // or stay at the last frame
    animation.frameIndex = animation.frameIndex - 1;
    trig(animation);
    if (depth == 1)
      m_state = AnimState::Stopped;
  }

  [[nodiscard]] static int getFps(const ObjectAnimation &animation) {
    if (animation.fps <= 0)
      return 10;
    return animation.fps;
  }

private:
  static void trig(const ObjectAnimation &animation) {
    if (!animation.callbacks.empty()) {
      animation.callbacks[animation.frameIndex]();
    }
  }

private:
  ObjectAnimation *m_anim{nullptr};
  AnimState m_state{AnimState::Pause};
  bool m_loop{false};
};
}