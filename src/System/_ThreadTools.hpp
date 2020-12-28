#pragma once
#include <ngf/Graphics/ImGuiExtensions.h>
#include "engge/Engine/ThreadBase.hpp"
#include <imgui.h>

namespace ng {
class _ThreadTools {
public:
  explicit _ThreadTools(Engine &engine) : _engine(engine) {}

  void render() {
    if (!ImGui::CollapsingHeader("Threads"))
      return;

    ImGui::Indent();
    const auto &threads = _engine.getThreads();
    for (const auto &thread : threads) {
      auto name = thread->getName();
      auto id = thread->getId();
      ImGui::PushID(id);
      auto type = thread->isGlobal() ? "global" : "local";
      auto isPauseable = thread->isPauseable();
      auto isSuspended = thread->isSuspended();
      auto isStopped = thread->isStopped();
      std::string state;
      if (isSuspended)
        state = "suspended";
      else if (isStopped)
        state = "stopped";
      else
        state = "playing";

      if (isSuspended) {
        if (ImGui::SmallButton("resume")) {
          thread->resume();
        }
        ImGui::SameLine();
      } else {
        if (ImGui::SmallButton("pause") && isPauseable) {
          thread->pause();
        }
        ImGui::SameLine();
      }
      if (ImGui::SmallButton("stop")) {
        thread->stop();
      }
      ImGui::SameLine();
      ImGui::Text("[%5d]: %-56s [%-6s] (%-9s)", id, name.data(), type, state.data());
      ImGui::PopID();
    }
    ImGui::Unindent();
  }

private:
  Engine &_engine;
};
}