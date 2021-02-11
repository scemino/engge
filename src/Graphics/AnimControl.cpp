#include <engge/Graphics/AnimControl.hpp>

namespace ng {
void AnimControl::setAnimation(Animation *anim) {
  m_anim = anim;
  stop();
}

Animation *AnimControl::getAnimation() { return m_anim; }

void AnimControl::play(bool loop) {
  m_loop = loop;
  if (!m_anim)
    return;
  m_anim->state = AnimState::Play;
  rewind(*m_anim);
}

void AnimControl::resume(bool loop) {
  m_loop = loop;
  if (!m_anim)
    return;
  m_anim->state = AnimState::Play;
}

void AnimControl::stop() {
  if (!m_anim)
    return;
  m_anim->state = AnimState::Stopped;
  resetAnim(*m_anim);
}

void AnimControl::pause() { m_anim->state = AnimState::Pause; }

AnimState AnimControl::getState() const {
  if (!m_anim)
    return AnimState::Stopped;
  return m_anim->state;
}

void AnimControl::update(const ngf::TimeSpan &e) {
  if (!m_anim)
    return;

  if (m_anim->state != AnimState::Play)
    return;

  if (m_anim->frames.empty() && m_anim->layers.empty())
    return;

  if (!m_anim->frames.empty()) {
    update(e, *m_anim);
    return;
  }

  bool isOver = true;
  for (auto &layer : m_anim->layers) {
    update(e, layer);
    isOver &= layer.state == ng::AnimState::Stopped;
  }
  if (isOver)
    m_anim->state = ng::AnimState::Stopped;
}

bool AnimControl::getLoop() const { return m_loop; }

void AnimControl::resetAnim(Animation &anim) {
  if (!anim.frames.empty()) {
    anim.state = ng::AnimState::Stopped;
    anim.frameIndex = static_cast<int>(anim.frames.size()) - 1;
  }
  if (!anim.layers.empty()) {
    std::for_each(anim.layers.begin(), anim.layers.end(), resetAnim);
  }
}

void AnimControl::rewind(Animation &anim) {
  if (!anim.frames.empty()) {
    anim.frameIndex = 0;
    anim.state = ng::AnimState::Play;
  }
  if (!anim.layers.empty()) {
    std::for_each(anim.layers.begin(), anim.layers.end(), rewind);
  }
}

void AnimControl::update(const ngf::TimeSpan &e, Animation &animation) const {
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
  if (animation.frameIndex != static_cast<int>(animation.frames.size())) {
    trig(animation);
    return;
  }

  // loop if requested
  if (m_loop || animation.loop) {
    animation.frameIndex = 0;
    return;
  }

  // or stay at the last frame
  animation.frameIndex = animation.frameIndex - 1;
  trig(animation);
  animation.state = AnimState::Stopped;
}

int AnimControl::getFps(const Animation &animation) {
  if (animation.fps <= 0)
    return 10;
  return animation.fps;
}

void AnimControl::trig(const Animation &animation) {
  if (animation.callbacks.empty() || animation.frameIndex < 0
      || animation.frameIndex >= static_cast<int>(animation.callbacks.size()))
    return;

  auto callback = animation.callbacks.at(animation.frameIndex);
  if (callback) {
    callback();
  }
}
}