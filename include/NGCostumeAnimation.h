#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "NGLayer.h"

namespace ng
{
enum class AnimationState
{
  Pause,
  Play
};

class NGCostumeAnimation : public sf::Drawable
{
public:
  NGCostumeAnimation(const std::string &name, sf::Texture &texture);
  ~NGCostumeAnimation();

  const std::string &getName() const { return _name; }
  std::vector<NGLayer *> &getLayers() { return _layers; }

  void play(bool loop = false);
  void pause() { _state = AnimationState::Pause; }
  bool isPlaying() const { return _state == AnimationState::Play; }

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  sf::Texture &_texture;
  std::string _name;
  std::vector<NGLayer *> _layers;
  AnimationState _state;
  bool _loop;
};
} // namespace ng
