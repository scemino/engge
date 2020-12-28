#include "engge/Entities/Actor/CostumeLayer.hpp"
#include "engge/Entities/Actor/Actor.hpp"
#include "engge/Room/Room.hpp"

namespace ng {
CostumeLayer::CostumeLayer(Animation &&animation)
    : _animation(animation) {
}

bool CostumeLayer::update(const ngf::TimeSpan &elapsed) {
  _animation.update(elapsed);
  return _animation.isPlaying();
}

void CostumeLayer::reset() {
  _animation.reset();
}

void CostumeLayer::draw(ngf::RenderTarget &target, ngf::RenderStates states) const {
  if (!getVisible())
    return;

  Animation anim(_animation);
  anim.setLeftDirection(_leftDirection);
  anim.setColor(_pActor->getRoom()->getAmbientLight());
  anim.draw(target, states);
}

} // namespace ng
