#include <imgui.h>
#include <vector>
#include "DebugControls.hpp"
#include <engge/Scripting/ScriptEngine.hpp>
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqstate.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"

namespace ng {
bool DebugControls::stringGetter(void *vec, int idx, const char **out_text) {
  auto &vector = *static_cast<std::vector<std::string> *>(vec);
  if (idx < 0 || idx >= static_cast<int>(vector.size())) {
    return false;
  }
  *out_text = vector.at(idx).data();
  return true;
}

void DebugControls::createTree(const char *tableKey, HSQOBJECT obj) {
  if (ImGui::TreeNode(tableKey, "%s = {}", tableKey)) {
    ImGui::PushID(tableKey);
    SQObjectPtr refpos;
    SQObjectPtr outkey, outvar;
    SQInteger res;
    while ((res = _table(obj)->Next(false, refpos, outkey, outvar)) != -1) {
      auto key = _stringval(outkey);
      ImGui::PushID(key);
      if (outvar._type == OT_STRING) {
        char value[32];
        strcpy(value, _stringval(outvar));
        ImGui::Text("%s = ", key);
        ImGui::SameLine(300);
        if (ImGui::InputText("", value, IM_ARRAYSIZE(value))) {
          ScriptEngine::set(obj, key, value);
        }
      } else if (outvar._type == OT_INTEGER) {
        auto value = static_cast<int>(_integer(outvar));
        ImGui::Text("%s = ", key);
        ImGui::SameLine(300);
        if (ImGui::InputInt("", &value)) {
          ScriptEngine::set(obj, key, value);
        }
      } else if (outvar._type == OT_BOOL) {
        auto value = _integer(outvar) != 0;
        ImGui::Text("%s = ", key);
        ImGui::SameLine(300);
        if (ImGui::Checkbox("", &value)) {
          ScriptEngine::set(obj, key, value);
        }
      } else if (outvar._type == OT_FLOAT) {
        auto value = _float(outvar);
        ImGui::Text("%s = ", key);
        ImGui::SameLine(300);
        if (ImGui::InputFloat("", &value)) {
          ScriptEngine::set(obj, key, value);
        }
      } else if (outvar._type == OT_CLOSURE || outvar._type == OT_NATIVECLOSURE) {
        ImGui::Text("%s = code()", key);
      } else if (outvar._type == OT_NULL) {
        ImGui::Text("%s = (null)", key);
      } else if (outvar._type == OT_TABLE) {
        createTree(key, outvar);
      } else {
        ImGui::Text("%s = (not managed, type = %d)", key, outvar._type);
      }
      refpos._type = OT_INTEGER;
      refpos._unVal.nInteger = res;
      ImGui::PopID();
    }
    ImGui::PopID();
    ImGui::TreePop();
  }
}
}