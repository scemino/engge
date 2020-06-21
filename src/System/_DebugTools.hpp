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
#include "Engine/EngineSettings.hpp"
#include "Entities/Objects/Object.hpp"
#include "Engine/Trigger.hpp"
#include "Audio/SoundId.hpp"
#include "_Util.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "Engine/InputStateConstants.hpp"
#include "../Engine/_DebugFeatures.hpp"
#include "_ConsoleTools.hpp"
#include "_TextureTools.hpp"
#include "_DebugControls.hpp"
#include "_ActorTools.hpp"
#include "_ObjectTools.hpp"
#include "_RoomTools.hpp"
#include "_SoundTools.hpp"
#include "_ThreadTools.hpp"
#include "_GeneralTools.hpp"
#include "_CameraTools.hpp"
#include "_PreferencesTools.hpp"

namespace ng {
class _DebugTools {
public:
  explicit _DebugTools(Engine &engine)
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

    renderTimes("Rendering (ms)", _renderTimes, []() { return _DebugFeatures::_renderTime; });
    renderTimes("Update (ms)", _updateTimes, []() { return _DebugFeatures::_updateTime; });
  }

  static void renderTimes(const char *label, Plot &plot, const std::function<sf::Time()> &func) {
    float average = 0.0f;
    for (int n = 0; n < IM_ARRAYSIZE(plot._values); n++)
      average += plot._values[n];
    average /= (float) IM_ARRAYSIZE(plot._values);
    char overlay[48];
    sprintf(overlay, "avg %f ms", average);

    plot._values[plot._offset] = func().asSeconds() * 1000.f;
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
      _DebugControls::createTree(pRoom->getName().c_str(), table);
    }
    ImGui::End();
  }

  void showGlobalsTable() {
    if (!_showGlobalsTable)
      return;

    ImGui::Begin("Globals table", &_showGlobalsTable);
    SQObjectPtr g;
    _table(ScriptEngine::getVm()->_roottable)->Get(ScriptEngine::toSquirrel("g"), g);
    _DebugControls::createTree("Globals", g);
    ImGui::End();
  }

  void showActorTable() {
    if (!_actorTools._showActorTable)
      return;

    ImGui::Begin("Actor table", &_actorTools._showActorTable);
    auto pActor = _engine.getCurrentActor();
    if (pActor) {
      auto table = pActor->getTable();
      _DebugControls::createTree(pActor->getName().c_str(), table);
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

private:
  Engine &_engine;
  bool _showRoomTable{false};
  bool _showGlobalsTable{false};
  _TextureTools _texturesTools;
  _ConsoleTools _consoleTools;
  _ActorTools _actorTools;
  _ObjectTools _objectTools;
  _RoomTools _roomTools;
  _SoundTools _soundTools;
  _ThreadTools _threadTools;
  _GeneralTools _generalTools;
  _CameraTools _cameraTools;
  _PreferencesTools _preferencesTools;
};
} // namespace ng
