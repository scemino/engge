#include "engge/Entities/Actor/CostumeAnimation.hpp"

namespace ng {

void CostumeAnimation::play(bool loop) {
  if (_loop == loop && _state == AnimationState::Play)
    return;
  _loop = loop;
  _state = AnimationState::Play;
  for (auto &layer : _layers) {
    layer.play(loop);
  }
}

void CostumeAnimation::update(const sf::Time &elapsed) {
  if (!isPlaying())
    return;
  auto loop = _loop;
  for (auto &layer : _layers) {
    loop |= layer.update(elapsed);
  }
  if (!loop) {
    pause();
  }
}

void CostumeAnimation::draw(sf::RenderTarget &target, sf::RenderStates states) const {
  for (const auto &layer : _layers) {
    target.draw(layer, states);
  }
}

bool CostumeAnimation::contains(const sf::Vector2f &pos) const {
  return std::any_of(_layers.cbegin(),
                     _layers.cend(),
                     [pos](const auto &layer) { return layer.getAnimation().contains(pos); });
}

} // namespace ng
