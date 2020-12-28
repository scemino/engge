#include "engge/Util/Util.hpp"

namespace ng {
ngf::frect getGlobalBounds(const ngf::Text &text) {
  return ngf::transform(text.getTransform().getTransform(), text.getLocalBounds());
}

ngf::frect getGlobalBounds(const ngf::Sprite &sprite) {
  return ngf::transform(sprite.getTransform().getTransform(), sprite.getLocalBounds());
}
}
