#pragma once
#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>

namespace ng {
class _Path : public ngf::Drawable {
public:
  explicit _Path(std::vector<glm::vec2> path);

  [[nodiscard]] const std::vector<glm::vec2> &getPath() const { return _path; }
  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  std::vector<glm::vec2> _path;
};
}
