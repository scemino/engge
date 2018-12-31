#pragma once
#include <sstream>
#include "SFML/Graphics.hpp"
#include "CostumeLayer.h"

namespace ng
{
enum class AnimationState
{
  Pause,
  Play
};

class CostumeAnimation : public sf::Drawable
{
public:
  CostumeAnimation(const std::string &name, sf::Texture &texture);
  ~CostumeAnimation();

  const std::string &getName() const { return _name; }
  std::vector<CostumeLayer *> &getLayers() { return _layers; }

  void play(bool loop = false);
  void pause() { _state = AnimationState::Pause; }
  bool isPlaying() const { return _state == AnimationState::Play; }

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  sf::Texture &_texture;
  std::string _name;
  std::vector<CostumeLayer *> _layers;
  AnimationState _state;
  bool _loop;
};
} // namespace ng
