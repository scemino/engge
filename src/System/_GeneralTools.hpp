#pragma once
#include <imgui-SFML.h>
#include <imgui.h>

namespace ng {
class _GeneralTools {
public:
  explicit _GeneralTools(Engine &engine, bool &textureVisible, bool &consoleVisible, bool &showGlobalsTable)
      : _engine(engine), _textureVisible(textureVisible), _consoleVisible(consoleVisible),
        _showGlobalsTable(showGlobalsTable) {}

  void render() {
    std::stringstream s;
    s << "Stack: " << sq_gettop(ScriptEngine::getVm());
    std::vector<std::string> stack;
    getStack(stack);
    ImGui::Combo(s.str().c_str(),
                 &_selectedStack,
                 _DebugControls::stringGetter,
                 static_cast<void *>(&stack),
                 stack.size());
    ImGui::Text("In cutscene: %s", _engine.inCutscene() ? "yes" : "no");
    auto dialogState = _engine.getDialogManager().getState();
    ImGui::Text("In dialog: %s",
                ((dialogState == DialogManagerState::Active)
                 ? "yes"
                 : (dialogState == DialogManagerState::WaitingForChoice ? "waiting for choice" : "no")));

    auto fade = _engine.getFadeAlpha();
    if (ImGui::SliderFloat("Fade", &fade, 0.f, 1.f, "%.1f", 0.1f)) {
      _engine.setFadeAlpha(fade);
    }
    auto gameSpeedFactor = _engine.getPreferences().getUserPreference(PreferenceNames::GameSpeedFactor,
                                                                      PreferenceDefaultValues::GameSpeedFactor);
    if (ImGui::SliderFloat("Game speed factor", &gameSpeedFactor, 0.f, 5.f)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::GameSpeedFactor, gameSpeedFactor);
    }
    ImGui::Checkbox("Show cursor position", &_DebugFeatures::showCursorPosition);
    ImGui::Checkbox("Show hovered object", &_DebugFeatures::showHoveredObject);
    ImGui::Checkbox("Textures", &_textureVisible);
    ImGui::Checkbox("Console", &_consoleVisible);
    ImGui::SameLine();
    if (ImGui::SmallButton("Globals...")) {
      _showGlobalsTable = true;
    }
  }

private:
  void getStack(std::vector<std::string> &stack) {
    HSQOBJECT obj;
    auto size = static_cast<int>(sq_gettop(ScriptEngine::getVm()));
    for (int i = 1; i <= size; ++i) {
      auto type = sq_gettype(ScriptEngine::getVm(), -i);
      sq_getstackobj(ScriptEngine::getVm(), -i, &obj);
      std::ostringstream s;
      s << "#" << i << ": ";
      switch (type) {
      case OT_NULL:s << "null";
        break;
      case OT_INTEGER:s << sq_objtointeger(&obj);
        break;
      case OT_FLOAT:s << sq_objtofloat(&obj);
        break;
      case OT_BOOL:s << (sq_objtobool(&obj) == SQTrue ? "true" : "false");
        break;
      case OT_USERPOINTER: {
        s << "userpointer";
        break;
      }
      case OT_STRING:s << sq_objtostring(&obj);
        break;
      case OT_TABLE: {
        int id;
        if (ScriptEngine::rawGet(obj, "_id", id)) {
          if (EntityManager::isActor(id)) {
            s << "actor";
          } else if (EntityManager::isRoom(id)) {
            s << "room";
          } else if (EntityManager::isObject(id)) {
            s << "object";
          } else if (EntityManager::isLight(id)) {
            s << "light";
          } else if (EntityManager::isSound(id)) {
            s << "sound";
          } else if (EntityManager::isThread(id)) {
            s << "thread table";
          } else {
            s << "table";
          }
        } else {
          s << "table";
        }
      }
        break;
      case OT_ARRAY:s << "array";
        break;
      case OT_CLOSURE: {
        s << "closure: ";
        auto pName = _closure(obj)->_function->_name;
        auto pSourcename = _closure(obj)->_function->_sourcename;
        auto line = _closure(obj)->_function->_lineinfos->_line;
        s << (pName._type != OT_NULL ? _stringval(pName) : "null");
        s << ' ' << (pSourcename._type != OT_NULL ? _stringval(pSourcename) : "-null");
        s << " [" << line << ']';
        break;
      }
      case OT_NATIVECLOSURE:s << "native closure";
        break;
      case OT_GENERATOR:s << "generator";
        break;
      case OT_USERDATA:s << "user data";
        break;
      case OT_THREAD: {
        s << "thread";
        auto pThread = EntityManager::getThreadFromVm(_thread(obj));
        if (pThread) {
          s << " " << pThread->getName();
        }
        break;
      }
      case OT_INSTANCE:s << "instance";
        break;
      case OT_WEAKREF:s << "weak ref";
        break;
      default:s << "?";
        break;
      }
      stack.push_back(s.str());
    }
  }

private:
  Engine &_engine;
  int _selectedStack{0};
  bool &_textureVisible;
  bool &_consoleVisible;
  bool &_showGlobalsTable;
};
}