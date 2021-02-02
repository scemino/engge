#pragma once
#include <optional>
#include <functional>
#include "squirrel.h"
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqcompiler.h"
#include "../../extlibs/squirrel/squirrel/sqfuncstate.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"
#include "engge/Engine/EngineSettings.hpp"
#include "engge/Entities/Objects/Object.hpp"
#include "engge/Engine/Trigger.hpp"
#include "engge/Audio/SoundId.hpp"
#include "../Util/Util.hpp"
#include <ngf/Graphics/ImGuiExtensions.h>
#include <imgui.h>
#include "engge/Engine/InputStateConstants.hpp"
#include "../Engine/DebugFeatures.hpp"
#include "ConsoleTools.hpp"
#include "TextureTools.hpp"
#include "DebugControls.hpp"
#include "ActorTools.hpp"
#include "ObjectTools.hpp"
#include "RoomTools.hpp"
#include "SoundTools.hpp"
#include "ThreadTools.hpp"
#include "GeneralTools.hpp"
#include "CameraTools.hpp"
#include "PreferencesTools.hpp"

namespace ng {
class DebugTools final {
public:
  explicit DebugTools(Engine &engine)
      : _engine(engine),
        _consoleTools(engine),
        _actorTools(engine),
        _objectTools(engine),
        _roomTools(engine),
        _soundTools(engine),
        _threadTools(engine),
        _generalTools(engine, _texturesTools._texturesVisible, _consoleTools._consoleVisible, _showGlobalsTable),
        _cameraTools(engine),
        _preferencesTools(engine) {
    memset(_renderTimes._values, 0, IM_ARRAYSIZE(_renderTimes._values));
    memset(_updateTimes._values, 0, IM_ARRAYSIZE(_updateTimes._values));
  }

  void render() {
    if (!visible)
      return;

    ImGui::Begin("Debug");

    _generalTools.render();
    _texturesTools.render();
    showGlobalsTable();
    _consoleTools.render();
    _cameraTools.render();
    showInputState();
    _preferencesTools.render();

    _actorTools.render();
    _objectTools.render();
    _roomTools.render();
    _soundTools.render();
    _threadTools.render();
    showRoomTable();
    showActorTable();
    showPerformance();

    ImGui::End();
  }

private:
  struct Plot {
    float _values[16];
    int _offset{0};
  } _renderTimes, _updateTimes;

  void showPerformance() {
    if (!ImGui::CollapsingHeader("Performance"))
      return;

    renderTimes("Rendering (ms)", _renderTimes, []() { return DebugFeatures::_renderTime; });
    renderTimes("Update (ms)", _updateTimes, []() { return DebugFeatures::_updateTime; });
  }

  static void renderTimes(const char *label, Plot &plot, const std::function<ngf::TimeSpan()> &func) {
    float average = 0.0f;
    for (int n = 0; n < IM_ARRAYSIZE(plot._values); n++)
      average += plot._values[n];
    average /= (float) IM_ARRAYSIZE(plot._values);
    char overlay[48];
    sprintf(overlay, "avg %f ms", average);

    plot._values[plot._offset] = func().getTotalSeconds() * 1000.f;
    ImGui::PlotLines(label, plot._values, IM_ARRAYSIZE(plot._values), plot._offset, overlay);
    plot._offset = (plot._offset + 1) % IM_ARRAYSIZE(plot._values);
  }

  void showRoomTable() {
    if (!_showRoomTable)
      return;

    ImGui::Begin("Room table", &_showRoomTable);
    auto pRoom = _engine.getRoom();
    if (pRoom) {
      auto table = pRoom->getTable();
      DebugControls::createTree(pRoom->getName().c_str(), table);
    }
    ImGui::End();
  }

  void showGlobalsTable() {
    if (!_showGlobalsTable)
      return;

    ImGui::Begin("Globals table", &_showGlobalsTable);
    SQObjectPtr g;
    _table(ScriptEngine::getVm()->_roottable)->Get(ScriptEngine::toSquirrel("g"), g);
    DebugControls::createTree("Globals", g);
    ImGui::End();
  }

  void showActorTable() {
    if (!_actorTools._showActorTable)
      return;

    ImGui::Begin("Actor table", &_actorTools._showActorTable);
    auto pActor = _engine.getCurrentActor();
    if (pActor) {
      auto table = pActor->getTable();
      DebugControls::createTree(pActor->getName().c_str(), table);
    }
    ImGui::End();
  }

  void showInputState() {
    if (!ImGui::CollapsingHeader("Input"))
      return;

    auto inputState = _engine.getInputState();
    auto inputActive = (inputState & InputStateConstants::UI_INPUT_ON) == InputStateConstants::UI_INPUT_ON;
    if (ImGui::Checkbox("Input active", &inputActive)) {
      _engine.setInputState(inputActive ? InputStateConstants::UI_INPUT_ON : InputStateConstants::UI_INPUT_OFF);
    }
    auto cursorVisible = (inputState & InputStateConstants::UI_CURSOR_ON) == InputStateConstants::UI_CURSOR_ON;
    if (ImGui::Checkbox("Cusrsor visible", &cursorVisible)) {
      _engine.setInputState(cursorVisible ? InputStateConstants::UI_CURSOR_ON
                                          : InputStateConstants::UI_CURSOR_OFF);
    }

    auto inputVerbs = (inputState & InputStateConstants::UI_VERBS_ON) == InputStateConstants::UI_VERBS_ON;
    if (ImGui::Checkbox("Input verbs", &inputVerbs)) {
      _engine.setInputState(inputVerbs ? InputStateConstants::UI_VERBS_ON : InputStateConstants::UI_VERBS_OFF);
    }
    auto inputHUD = (inputState & InputStateConstants::UI_HUDOBJECTS_ON) == InputStateConstants::UI_HUDOBJECTS_ON;
    if (ImGui::Checkbox("Input HUD", &inputHUD)) {
      _engine.setInputState(inputHUD ? InputStateConstants::UI_HUDOBJECTS_ON
                                     : InputStateConstants::UI_HUDOBJECTS_OFF);
    }
  }

public:
  bool visible{false};

private:
  Engine &_engine;
  bool _showRoomTable{false};
  bool _showGlobalsTable{false};
  TextureTools _texturesTools;
  ConsoleTools _consoleTools;
  ActorTools _actorTools;
  ObjectTools _objectTools;
  RoomTools _roomTools;
  SoundTools _soundTools;
  ThreadTools _threadTools;
  GeneralTools _generalTools;
  CameraTools _cameraTools;
  PreferencesTools _preferencesTools;
};
} // namespace ng
