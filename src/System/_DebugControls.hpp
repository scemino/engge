#pragma once
#include <vector>
#include "imgui-SFML.h"
#include "imgui.h"

namespace ng {
class _DebugControls {
public:
  static bool stringGetter(void *vec, int idx, const char **out_text) {
    auto &vector = *static_cast<std::vector<std::string> *>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) {
      return false;
    }
    *out_text = vector.at(idx).data();
    return true;
  }

  static bool ColorEdit4(const char *label, sf::Color &color) {
    float imColor[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
    if (ImGui::ColorEdit4(label, imColor)) {
      color.r = static_cast<sf::Uint8>(imColor[0] * 255.f);
      color.g = static_cast<sf::Uint8>(imColor[1] * 255.f);
      color.b = static_cast<sf::Uint8>(imColor[2] * 255.f);
      color.a = static_cast<sf::Uint8>(imColor[3] * 255.f);
      return true;
    }
    return false;
  }

  static bool InputInt2(const char *label, sf::Vector2i &vector) {
    int vec[2] = {vector.x, vector.y};
    if (ImGui::InputInt2(label, vec)) {
      vector.x = vec[0];
      vector.y = vec[1];
      return true;
    }
    return false;
  }

  static bool InputInt4(const char *label, sf::IntRect &rect) {
    int r[4] = {rect.left, rect.top, rect.width, rect.height};
    if (ImGui::InputInt4(label, r)) {
      rect.left = r[0];
      rect.top = r[1];
      rect.width = r[2];
      rect.height = r[3];
      return true;
    }
    return false;
  }

  static bool InputFloat2(const char *label, sf::Vector2f &vector) {
    float vec[2] = {vector.x, vector.y};
    if (ImGui::InputFloat2(label, vec)) {
      vector.x = vec[0];
      vector.y = vec[1];
      return true;
    }
    return false;
  }

  static void createTree(const char *tableKey, HSQOBJECT obj) {
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
};
}
