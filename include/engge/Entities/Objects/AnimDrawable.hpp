#pragma once
#include <memory>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/Sprite.h>
#include <ngf/Graphics/Texture.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Math/Transform.h>
#include <engge/Entities/Objects/ObjectAnimation.hpp>
#include <engge/Graphics/LightingShader.h>

namespace ng {
class AnimDrawable {
public:
  void setAnim(ObjectAnimation *anim) { m_anim = anim; }
  void setFlipX(bool flipX) { m_flipX = flipX; }
  void setColor(const ngf::Color color) { m_color = color; }

  void draw(const glm::vec2 &pos, ngf::RenderTarget &target, ngf::RenderStates states) const {
    if (!m_anim)
      return;

    if (m_anim->frames.empty() && m_anim->layers.empty())
      return;

    draw(pos, *m_anim, target, states);

    for (const auto &layer : m_anim->layers) {
      draw(pos, layer, target, states);
    }
  }

private:
  void draw(const glm::vec2 &pos,
            const ObjectAnimation &anim,
            ngf::RenderTarget &target,
            ngf::RenderStates states) const {
    if (!anim.visible)
      return;
    if (anim.frames.empty())
      return;

    glm::ivec2 offset{0, 0};
    if (!anim.offsets.empty()) {
      offset = anim.offsets[anim.frameIndex];
    }
    const auto frame = anim.frames[anim.frameIndex];
    glm::vec2 origin = {frame.sourceSize.x / 2.f, frame.sourceSize.y / 2.f};

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
    auto texture = anim.texture;
    if(!texture)
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

private:
  ObjectAnimation *m_anim{nullptr};
  bool m_flipX{false};
  ngf::Color m_color{ngf::Colors::White};
};
}