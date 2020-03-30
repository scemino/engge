#pragma once
#include <functional>
#include "SFML/Graphics.hpp"

namespace ng {

class AnimationFrame {
public:
  using Callback = std::function<void()>;

public:
  explicit AnimationFrame(sf::IntRect rect, Callback callback = nullptr);

  void setName(const std::string &name);
  [[nodiscard]] const std::string &getName() const;

  void setRect(sf::IntRect rect);
  [[nodiscard]] sf::IntRect getRect(bool leftDirection) const;
  [[nodiscard]] sf::Vector2f getOrigin(bool leftDirection) const;

  void setSourceRect(sf::IntRect rect) { _sourceRect = rect; }
  [[nodiscard]] sf::IntRect getSourceRect() const { return _sourceRect; }

  void setOffset(sf::Vector2f offset) { _offset = offset; }
  [[nodiscard]] sf::Vector2f getOffset(bool leftDirection) const;

  void setSize(sf::Vector2i size) { _size = size; }
  [[nodiscard]] sf::Vector2i getSize() const { return _size; }

  void setCallback(Callback callback);
  void call();

private:
  std::string _name;
  sf::IntRect _rect;
  sf::IntRect _sourceRect;
  sf::Vector2i _size;
  sf::Vector2f _offset;
  Callback _callback{nullptr};
};

} // namespace ng
