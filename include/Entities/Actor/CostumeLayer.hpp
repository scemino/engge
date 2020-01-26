#pragma once
#include <sstream>
#include <optional>
#include "SFML/Graphics.hpp"
#include "../Objects/Animation.hpp"

namespace ng
{
class Actor;

class CostumeLayer : public sf::Drawable
{
public:
  explicit CostumeLayer(Animation&& animation);
  
  void setName(const std::string &name) { _name = name; }
  const std::string &getName() const { return _name; }

  int getFlags() const { return _flags; }
  void setFlags(int flags) { _flags = flags; }

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  bool getVisible() const { return _isVisible; }

  void reset();
  void setActor(Actor *pActor) { _pActor = pActor; }
  
  Animation& getAnimation() { return _animation; }
  void setLoop(bool loop) { _loop = loop; }
  bool getLoop() const { return _loop; }
  void play(bool loop = false) { _animation.play(_loop || loop); }
  
  void setLeftDirection(bool leftDirection) { _leftDirection = leftDirection; }

  bool update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
  void updateTrigger();
  void updateSoundTrigger();

private:
  Animation _animation;
  std::string _name;
  int _flags{0};
  bool _isVisible{true};
  Actor *_pActor{nullptr};
  bool _loop{false};
  bool _leftDirection{false};
};
} // namespace ng