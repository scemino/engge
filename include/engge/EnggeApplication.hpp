#pragma once
#include <ngf/Application.h>
#include <ngf/System/Event.h>
#include <ngf/System/TimeSpan.h>
#include <ngf/Graphics/RenderTarget.h>
#include <ngf/Graphics/RenderStates.h>
#include <engge/System/Services.hpp>
#include <engge/Engine/Camera.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <glm/vec2.hpp>
#include "System/DebugTools/DebugTools.hpp"

namespace ng {

class Engine;

class EnggeApplication : public ngf::Application {
private:
  void onInit() override;
  void onEvent(ngf::Event &event) override;
  void onRender(ngf::RenderTarget &target) override;
  void onImGuiRender() override;
  void onUpdate(const ngf::TimeSpan &elapsed) override;
  void onQuit() override;

private:
  ng::Engine *m_engine{nullptr};
  bool m_init{false};
  glm::ivec2 m_pos;
  bool m_isMousePressed{false};
  bool m_isKeyPressed{false};
  std::unique_ptr<DebugTools> m_debugTools;
};
}
