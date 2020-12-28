#pragma once
#include <memory>
#include <string>
#include <vector>
#include "CostumeLayer.hpp"

namespace ng {
enum class AnimationState {
  Pause,
  Play
};

class CostumeAnimation : public ngf::Drawable {
public:
  void setName(const std::string &name) { _name = name; }
  [[nodiscard]] const std::string &getName() const { return _name; }

  std::vector<CostumeLayer> &getLayers() { return _layers; }

  void play(bool loop = false);
  void pause() { _state = AnimationState::Pause; }
  [[nodiscard]] bool isPlaying() const { return _state == AnimationState::Play; }

  void setFlags(int flags) { _flags = flags; }
  [[nodiscard]] int getFlags() const { return _flags; }

  [[nodiscard]] bool contains(const glm::vec2 &pos) const;

  void update(const ngf::TimeSpan &elapsed);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  std::string _name;
  std::vector<CostumeLayer> _layers;
  AnimationState _state{AnimationState::Pause};
  bool _loop{false};
  int _flags{0};
};
} // namespace ng
