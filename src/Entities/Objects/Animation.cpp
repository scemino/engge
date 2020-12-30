#include <utility>
#include <ngf/Math/Transform.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/LightingShader.h>
#include "engge/Graphics/ResourceManager.hpp"
#include "engge/Entities/Objects/Animation.hpp"
#include "engge/System/Locator.hpp"

namespace ng {

Animation::Animation() = default;

Animation::Animation(std::string texture, std::string name)
    : _texture(std::move(texture)), _name(std::move(name)) {
}

Animation::~Animation() = default;

size_t Animation::size() const noexcept { return _frames.size(); }

bool Animation::empty() const noexcept { return _frames.empty(); }

void Animation::addFrame(AnimationFrame &&frame) {
  _frames.push_back(std::move(frame));
}

void Animation::reset() {
  if (_frames.empty())
    return;
  _index = _frames.size() - 1;
}

void Animation::play(bool loop) {
  _loop = loop;
  _state = AnimState::Play;
  _index = 0;
}

void Animation::update(const ngf::TimeSpan &elapsed) {
  if (_state == AnimState::Pause)
    return;

  if (_frames.empty())
    return;

  _time += elapsed;
  if (_time.getTotalSeconds() > (1.f / static_cast<float>(_fps))) {
    _time = ngf::TimeSpan::seconds(_time.getTotalSeconds() - (1.f / static_cast<float>(_fps)));
    if (_loop || _index != _frames.size() - 1) {
      _index = (_index + 1) % _frames.size();
      _frames.at(_index).call();
    } else {
      pause();
    }
  }
}

AnimationFrame &Animation::at(size_t index) {
  return _frames.at(index);
}

void Animation::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (_frames.empty())
    return;

  const auto &frame = _frames.at(_index);
  auto rect = frame.getRect();
  auto origin = frame.getOrigin();
  auto pos = frame.getPosition(_leftDirection);
  auto offset = frame.getOffset(_leftDirection);

  ngf::Transform tFlipX;
  tFlipX.setScale({_leftDirection ? -1 : 1, 1});
  ngf::Transform t;
  t.setOrigin(origin);
  t.setPosition(pos);
  t.move(offset);
  states.transform = tFlipX.getTransform() * t.getTransform() * states.transform;

//  auto texture = Locator<ResourceManager>::get().getTexture(_texture);
//
//  auto pShader = (LightingShader *) states.shader;
//  auto texSize = texture->getSize();
//  pShader->setTexture(*texture);
//  pShader->setContentSize(frame.sourceSize);
//  pShader->setSpriteOffset({-frame.frame.getWidth() / 2.f + pos.x, -frame.frame.getHeight() / 2.f - pos.y});
//  pShader->setSpritePosInSheet({static_cast<float>(frame.frame.min.x) / texSize.x,
//                                static_cast<float>(frame.frame.min.y) / texSize.y});
//  pShader->setSpriteSizeRelToSheet({static_cast<float>(frame.sourceSize.x) / texSize.x,
//                                    static_cast<float>(frame.sourceSize.y) / texSize.y});

  ngf::Sprite sprite(*Locator<ResourceManager>::get().getTexture(_texture), rect);
  sprite.setColor(_color);
  sprite.draw(target, states);
}

bool Animation::contains(const glm::vec2 &pos) const {
  if (_frames.empty())
    return false;

  const auto &frame = _frames.at(_index);

  ngf::Transform t;
  t.setOrigin(frame.getOrigin());
  t.setPosition(frame.getPosition(_leftDirection));
  t.move(frame.getOffset(_leftDirection));

  auto rect = frame.getRect();
  ngf::frect r1 = ngf::frect::fromPositionSize({0, 0}, rect.getSize());
  auto r = ngf::transform(t.getTransform(), r1);
  return r.contains(pos);
}

} // namespace ng
