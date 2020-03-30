#pragma once
#include <sstream>
#include <optional>
#include "SFML/Graphics.hpp"
#include "../Objects/Animation.hpp"

namespace ng {
class Actor;

class CostumeLayer : public sf::Drawable {
public:
  explicit CostumeLayer(Animation &&animation);

  void setName(const std::string &name) { _name = name; }
  [[nodiscard]] const std::string &getName() const { return _name; }

  [[nodiscard]] int getFlags() const { return _flags; }
  void setFlags(int flags) { _flags = flags; }

  void setVisible(bool isVisible) { _isVisible = isVisible; }
  [[nodiscard]] bool getVisible() const { return _isVisible; }

  void reset();
  void setActor(Actor *pActor) { _pActor = pActor; }

  Animation &getAnimation() { return _animation; }
  [[nodiscard]] const Animation &getAnimation() const { return _animation; }

  void setLoop(bool loop) { _loop = loop; }
  [[nodiscard]] bool getLoop() const { return _loop; }
  void play(bool loop = false) { _animation.play(_loop || loop); }

  void setLeftDirection(bool leftDirection) { _leftDirection = leftDirection; }

  bool update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

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