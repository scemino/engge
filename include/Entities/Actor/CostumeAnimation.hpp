#pragma once
#include <memory>
#include <sstream>
#include "SFML/Graphics.hpp"
#include "CostumeLayer.hpp"

namespace ng {
enum class AnimationState {
  Pause,
  Play
};

class CostumeAnimation : public sf::Drawable {
public:
  void setName(const std::string &name) { _name = name; }
  [[nodiscard]] const std::string &getName() const { return _name; }

  std::vector<CostumeLayer> &getLayers() { return _layers; }

  void play(bool loop = false);
  void pause() { _state = AnimationState::Pause; }
  [[nodiscard]] bool isPlaying() const { return _state == AnimationState::Play; }

  void setFlags(int flags) { _flags = flags; }
  [[nodiscard]] int getFlags() const { return _flags; }

  [[nodiscard]] bool contains(const sf::Vector2f &pos) const;

  void update(const sf::Time &elapsed);

private:
  void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

private:
  std::string _name;
  std::vector<CostumeLayer> _layers;
  AnimationState _state{AnimationState::Pause};
  bool _loop{false};
  int _flags{0};
};
} // namespace ng
