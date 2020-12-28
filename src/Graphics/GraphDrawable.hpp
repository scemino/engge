#pragma once
#include <vector>
#include <memory>
#include <ngf/Graphics/Drawable.h>
#include <ngf/Graphics/RenderStates.h>
#include <ngf/Graphics/RenderTarget.h>

namespace ngf {
  class Graph;
}

namespace ng {

class GraphDrawable : public ngf::Drawable {
public:
  explicit GraphDrawable(const ngf::Graph &graph, int height);

  void draw(ngf::RenderTarget &target, ngf::RenderStates states) const override;

private:
  const ngf::Graph &m_graph;
  const int m_height;
};

} // namespace ng
