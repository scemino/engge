#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "GGLayer.h"

namespace gg
{
enum class AnimationState
{
  Pause,
  Play
};

class GGCostumeAnimation : public sf::Drawable
{
public:
  GGCostumeAnimation(const std::string &name, sf::Texture &texture);
  ~GGCostumeAnimation();

  const std::string &getName() const { return _name; }
  std::vector<GGLayer *> &getLayers() { return _layers; }

  void play(bool loop = false);
  void pause() { _state = AnimationState::Pause; }
  bool isPlaying() const { return _state == AnimationState::Play; }

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  sf::Texture &_texture;
  std::string _name;
  std::vector<GGLayer *> _layers;
  AnimationState _state;
  bool _loop;
};
} // namespace gg
