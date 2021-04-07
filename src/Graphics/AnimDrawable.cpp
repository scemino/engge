#include <engge/Graphics/AnimDrawable.hpp>
#include <engge/Graphics/Animation.hpp>
#include <engge/Graphics/LightingShader.h>
#include <ngf/Math/Transform.h>
#include <ngf/Graphics/Sprite.h>
#include <engge/Graphics/ResourceManager.hpp>
#include <engge/System/Locator.hpp>

namespace ng {
void AnimDrawable::setAnim(const Animation *anim) { m_anim = anim; }

void AnimDrawable::setFlipX(bool flipX) { m_flipX = flipX; }

void AnimDrawable::setColor(const ngf::Color &color) { m_color = color; }

void AnimDrawable::draw(const glm::vec2 &pos, ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!m_anim)
    return;

  if (m_anim->frames.empty() && m_anim->layers.empty())
    return;

  draw(pos, *m_anim, target, states);

  for (const auto &layer : m_anim->layers) {
    draw(pos, layer, target, states);
  }
}

void AnimDrawable::draw(const glm::vec2 &pos,
                        const Animation &anim,
                        ngf::RenderTarget &target,
                        ngf::RenderStates states) const {
  if (!anim.visible)
    return;
  if (anim.frames.empty())
    return;

  glm::ivec2 offset{0, 0};
  if (!anim.offsets.empty() && anim.frameIndex < static_cast<int>(anim.offsets.size())) {
    offset = anim.offsets.at(anim.frameIndex);
  }
  const auto frame = anim.frames.at(anim.frameIndex);
  if (frame.isNull)
    return;

  glm::vec2 origin = {static_cast<int>(frame.sourceSize.x / 2.f), static_cast<int>((frame.sourceSize.y + 1) / 2.f)};

  glm::vec2 off = {m_flipX ? frame.sourceSize.x - frame.spriteSourceSize.min.x - offset.x :
                   frame.spriteSourceSize.min.x + offset.x,
                   frame.spriteSourceSize.min.y - offset.y};
  ngf::Transform t;
  t.setPosition(off);
  t.setOrigin(origin);

  // flip X if actor goes left
  ngf::Transform tFlipX;
  tFlipX.setScale({m_flipX ? -1 : 1, 1});
  states.transform = tFlipX.getTransform() * t.getTransform() * states.transform;

  auto pShader = (LightingShader *) states.shader;
  auto texture = Locator<ResourceManager>::get().getTexture(anim.texture);
  if (!texture)
    return;

  auto texSize = texture->getSize();
  pShader->setTexture(*texture);
  pShader->setContentSize(frame.sourceSize);
  pShader->setSpriteOffset({-frame.frame.getWidth() / 2.f + pos.x, -frame.frame.getHeight() / 2.f - pos.y});
  pShader->setSpritePosInSheet({static_cast<float>(frame.frame.min.x) / texSize.x,
                                static_cast<float>(frame.frame.min.y) / texSize.y});
  pShader->setSpriteSizeRelToSheet({static_cast<float>(frame.sourceSize.x) / texSize.x,
                                    static_cast<float>(frame.sourceSize.y) / texSize.y});

  ngf::Sprite sprite(*texture, frame.frame);
  sprite.setColor(m_color);
  sprite.draw(target, states);
}
}