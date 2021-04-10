#include "DebugTools.hpp"
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Engine/InputStateConstants.hpp>
#include "Engine/DebugFeatures.hpp"
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"

namespace ng {
DebugTools::DebugTools(Engine &engine)
    : m_engine(engine),
      m_consoleTools(engine),
      m_actorTools(engine),
      m_objectTools(engine),
      m_roomTools(engine),
      m_soundTools(engine),
      m_threadTools(engine),
      m_generalTools(engine,
                     m_texturesTools.texturesVisible,
                     m_consoleTools.consoleVisible,
                     m_showGlobalsTable,
                     m_soundTools.soundsVisible,
                     m_threadTools.threadsVisible,
                     m_actorTools.actorsVisible,
                     m_objectTools.objectsVisible),
      m_cameraTools(engine),
      m_preferencesTools(engine) {
  memset(m_renderTimes.values, 0, IM_ARRAYSIZE(m_renderTimes.values));
  memset(m_updateTimes.values, 0, IM_ARRAYSIZE(m_updateTimes.values));
}

void DebugTools::render() {
  if (!visible)
    return;

  ImGui::Begin("Debug");

  m_generalTools.render();
  m_texturesTools.render();
  showGlobalsTable();
  m_consoleTools.render();
  m_cameraTools.render();
  showInputState();
  m_preferencesTools.render();

  m_actorTools.render();
  m_objectTools.render();
  m_roomTools.render();
  m_soundTools.render();
  m_threadTools.render();
  showRoomTable();
  showPerformance();

  ImGui::End();
}

void DebugTools::showPerformance() {
  if (!ImGui::CollapsingHeader("Performance"))
    return;

  renderTimes("Rendering (ms)", m_renderTimes, []() { return DebugFeatures::renderTime; });
  renderTimes("Update (ms)", m_updateTimes, []() { return DebugFeatures::updateTime; });
}

void DebugTools::renderTimes(const char *label, Plot &plot, const std::function<ngf::TimeSpan()> &func) {
  float average = 0.0f;
  for (int n = 0; n < IM_ARRAYSIZE(plot.values); n++)
    average += plot.values[n];
  average /= (float) IM_ARRAYSIZE(plot.values);
  char overlay[48];
  sprintf(overlay, "avg %f ms", average);

  plot.values[plot.offset] = func().getTotalSeconds() * 1000.f;
  ImGui::PlotLines(label, plot.values, IM_ARRAYSIZE(plot.values), plot.offset, overlay);
  plot.offset = (plot.offset + 1) % IM_ARRAYSIZE(plot.values);
}

void DebugTools::showRoomTable() {
  if (!m_showRoomTable)
    return;

  ImGui::Begin("Room table", &m_showRoomTable);
  auto pRoom = m_engine.getRoom();
  if (pRoom) {
    auto table = pRoom->getTable();
    DebugControls::createTree(pRoom->getName().c_str(), table);
  }
  ImGui::End();
}

void DebugTools::showGlobalsTable() {
  if (!m_showGlobalsTable)
    return;

  ImGui::Begin("Globals table", &m_showGlobalsTable);
  SQObjectPtr g;
  _table(ScriptEngine::getVm()->_roottable)->Get(ScriptEngine::toSquirrel("g"), g);
  DebugControls::createTree("Globals", g);
  ImGui::End();
}

void DebugTools::showInputState() {
  if (!ImGui::CollapsingHeader("Input"))
    return;

  auto inputState = m_engine.getInputState();
  auto inputActive = (inputState & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON;
  if (ImGui::Checkbox("Input active", &inputActive)) {
    m_engine.setInputState(inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
  }
  auto cursorVisible = (inputState & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON;
  if (ImGui::Checkbox("Cusrsor visible", &cursorVisible)) {
    m_engine.setInputState(cursorVisible ? InputStateConstants::UI_CURSOR_ON
                                         : InputStateConstants::UI_CURSOR_OFF);
  }

  auto inputVerbs = (inputState & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON;
  if (ImGui::Checkbox("Input verbs", &inputVerbs)) {
    m_engine.setInputState(inputVerbs ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
  }
  auto inputHUD = (inputState & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON;
  if (ImGui::Checkbox("Input HUD", &inputHUD)) {
    m_engine.setInputState(inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON
                                    : InputStateConstants::UI_HUDOBJECTS_OFF);
  }
}
}