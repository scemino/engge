#include "ThreadTools.hpp"
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/ThreadBase.hpp>
#include <imgui.h>
#include <string>

namespace ng {
namespace {
std::string getType(ThreadBase *thread) {
  return thread->isGlobal() ? "global" : "local";
}

std::string getState(ThreadBase *thread) {
  if (thread->isSuspended())
    return "suspended";
  if (thread->isStopped())
    return "stopped";
  return "playing";
}

void showControls(ThreadBase *thread) {
  if (thread->isSuspended()) {
    if (ImGui::SmallButton("resume")) {
      thread->resume();
    }
    ImGui::SameLine();
  } else {
    if (ImGui::SmallButton("pause") && thread->isPauseable()) {
      thread->pause();
    }
    ImGui::SameLine();
  }
  if (ImGui::SmallButton("stop")) {
    thread->stop();
  }
}
}

ThreadTools::ThreadTools(Engine &engine) : m_engine(engine) {}

void ThreadTools::render() {
  if (!threadsVisible)
    return;

  const auto &threads = m_engine.getThreads();
  ImGui::Begin("Threads", &threadsVisible);
  ImGui::Text("# threads: %lu", threads.size());
  ImGui::Separator();

  if (ImGui::BeginTable("Threads",
                        5,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable
                            | ImGuiTableFlags_RowBg)) {
    ImGui::TableSetupColumn("");
    ImGui::TableSetupColumn("Id");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Type");
    ImGui::TableSetupColumn("State");
    ImGui::TableHeadersRow();

    for (const auto &thread : threads) {
      const auto name = thread->getName();
      const auto id = thread->getId();
      const auto type = getType(thread.get());
      const auto state = getState(thread.get());

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      showControls(thread.get());
      ImGui::TableNextColumn();
      ImGui::Text("%5d", id);
      ImGui::TableNextColumn();
      ImGui::Text("%-56s", name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%-6s", type.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%-9s", state.c_str());
    }
    ImGui::EndTable();
  }
  ImGui::End();
}
}