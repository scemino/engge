#include <optional>
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
#include "_Util.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "Engine/InputStateConstants.hpp"

namespace ng {
class _DebugTools {
public:
  explicit _DebugTools(Engine &engine) : _engine(engine) {
  }

  void render() {
    ImGui::Begin("Debug");
    std::stringstream s;
    s << "Stack: " << sq_gettop(ScriptEngine::getVm());
    std::vector<std::string> stack;
    getStack(stack);
    ImGui::Combo(s.str().c_str(), &_selectedStack, stringGetter, static_cast<void *>(&stack), stack.size());
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

    showCamera();
    showInputState();
    showPrefs();
    showScript();

    showActors();
    showObjects();
    showRooms();
    showSounds();

    ImGui::End();
  }

private:
  void showLayers() {
    ImGui::Columns(4, "LayersColumns", false);
    ImGui::Separator();
    for (auto &layer : _pSelectedAnim->getLayers()) {
      ImGui::Text("Layer %s", layer.getName().c_str());
      ImGui::NextColumn();
      auto &anim = layer.getAnimation();
      auto animIndex = static_cast<int>(anim.getIndex());
      auto animSize = static_cast<int>(anim.size() - 1);
      if (ImGui::SliderInt("Index", &animIndex, 0, animSize)) {
        anim.setIndex(animIndex);
      }
      ImGui::NextColumn();
      auto layerVisible = layer.getVisible();
      if (ImGui::Checkbox("Visible", &layerVisible)) {
        layer.setVisible(layerVisible);
      }
      ImGui::NextColumn();
      auto layerLoop = layer.getLoop();
      if (ImGui::Checkbox("Loop", &layerLoop)) {
        layer.setLoop(layerLoop);
      }
      ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();
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

  void showPrefs() {
    if (!ImGui::CollapsingHeader("Preferences"))
      return;

    auto selectedLang = getSelectedLang();
    if (ImGui::Combo("Language", &selectedLang, _langs, 5)) {
      setSelectedLang(selectedLang);
    }
    auto retroVerbs = _engine.getPreferences().getUserPreference(PreferenceNames::RetroVerbs,
                                                                 PreferenceDefaultValues::RetroVerbs);
    if (ImGui::Checkbox("Retro Verbs", &retroVerbs)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::RetroVerbs, retroVerbs);
    }
    auto retroFonts = _engine.getPreferences().getUserPreference(PreferenceNames::RetroFonts,
                                                                 PreferenceDefaultValues::RetroFonts);
    if (ImGui::Checkbox("Retro Fonts", &retroFonts)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::RetroFonts, retroFonts);
    }
    auto invertVerbHighlight =
        _engine.getPreferences().getUserPreference(PreferenceNames::InvertVerbHighlight,
                                                   PreferenceDefaultValues::InvertVerbHighlight);
    if (ImGui::Checkbox("Invert Verb Highlight", &invertVerbHighlight)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::InvertVerbHighlight, invertVerbHighlight);
    }
    auto hudSentence = _engine.getPreferences().getUserPreference(PreferenceNames::HudSentence,
                                                                  PreferenceDefaultValues::HudSentence);
    if (ImGui::Checkbox("HUD Sentence", &hudSentence)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::HudSentence, hudSentence);
    }
    auto uiBackingAlpha = _engine.getPreferences().getUserPreference(PreferenceNames::UiBackingAlpha,
                                                                     PreferenceDefaultValues::UiBackingAlpha) * 100.f;
    if (ImGui::SliderFloat("UI Backing Alpha", &uiBackingAlpha, 0.f, 100.f)) {
      _engine.getPreferences().setUserPreference(PreferenceNames::UiBackingAlpha, uiBackingAlpha * 0.01f);
    }
  }

  void showScript() {
    if (!ImGui::CollapsingHeader("Script"))
      return;

    static char str0[1024] = "";
    ImGui::InputText("Input text", str0, IM_ARRAYSIZE(str0));
    ImGui::SameLine();
    if (ImGui::Button("Send")) {
      _engine.execute(str0);
    }
  }

  void showCamera() {
    if (!ImGui::CollapsingHeader("Camera"))
      return;

    ImGui::Text("Is moving: %s", _engine.getCamera().isMoving() ? "yes" : "no");
    sf::Vector2f pos = _engine.getCamera().getAt();
    if (InputFloat2("Position", pos)) {
      _engine.getCamera().at(pos);
    }
    auto optBounds = _engine.getCamera().getBounds();
    sf::IntRect bounds;
    if (optBounds.has_value()) {
      bounds = optBounds.value();
    }
    if (InputInt4("Bounds", bounds)) {
      _engine.getCamera().setBounds(bounds);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset##Bounds")) {
      _engine.getCamera().resetBounds();
    }
  }

  int getSelectedLang() {
    auto lang =
        _engine.getPreferences().getUserPreference(PreferenceNames::Language, PreferenceDefaultValues::Language);
    for (size_t i = 0; i < 5; ++i) {
      if (!strcmp(lang.c_str(), _langs[i]))
        return i;
    }
    return 0;
  }

  void setSelectedLang(int lang) {
    _engine.getPreferences().setUserPreference(PreferenceNames::Language, std::string(_langs[lang]));
  }

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
          if (ResourceManager::isActor(id)) {
            s << "actor";
          } else if (ResourceManager::isRoom(id)) {
            s << "room";
          } else if (ResourceManager::isObject(id)) {
            s << "object";
          } else if (ResourceManager::isLight(id)) {
            s << "light";
          } else if (ResourceManager::isSound(id)) {
            s << "sound";
          } else if (ResourceManager::isThread(id)) {
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
        auto pThread = ScriptEngine::getThreadFromVm(_thread(obj));
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

  void showSounds() {
    if (!ImGui::CollapsingHeader("Sounds"))
      return;

    ImGui::Indent();
    auto &sounds = _engine.getSoundManager().getSounds();
    for (auto i = 0; i < static_cast<int>(sounds.size()); i++) {
      const auto &sound = sounds[i];
      const auto *sd = sound ? sound->getSoundDefinition(): nullptr;
      const auto* name = !sd ? "<free>" : sd->getPath().data();
      auto loopTimes = sound ? sound->getLoopTimes() : 0;
      auto volume = sound ? sound->getVolume() : 0.f;
      std::string category;
      ImVec4 catColor = ImVec4(sf::Color::White);
      if(sound) {
        switch(sound->getSoundCategory()){
        case SoundCategory::Music:
          catColor = ImVec4(sf::Color::Green);
          category = "[music]";
          break;
        case SoundCategory::Sound:
          catColor = ImVec4(sf::Color::Red);
          category = "[sound]";
          break;
        case SoundCategory::Talk:
          catColor = ImVec4(sf::Color::Yellow);
          category = "[talk]";
          break;
        }
      }
      if(ImGui::SmallButton("stop") && sound){
        sound->stop();
      }
      ImGui::SameLine();
      ImGui::Text("%2d: %-48s x%2d", i, name, loopTimes);
      ImGui::SameLine();
      ImGui::TextColored(catColor, " %7s", category.data());
      ImGui::SameLine();
      ImGui::Text(" %.1f", volume);
    }
    ImGui::Unindent();
  }

  void showActors() {
    if (!ImGui::CollapsingHeader("Actors"))
      return;

    ImGui::Indent();
    auto &actors = _engine.getActors();
    _actorInfos.clear();
    for (auto &&actor : actors) {
      _actorInfos.push_back(toUtf8(actor->getTranslatedName()));
    }
    ImGui::Combo("##Actor", &_selectedActor, stringGetter, static_cast<void *>(&_actorInfos), _actorInfos.size());
    auto &actor = actors[_selectedActor];

    auto head = actor->getCostume().getHeadIndex();
    if (ImGui::SliderInt("Head index", &head, 0, 5)) {
      actor->getCostume().setHeadIndex(head);
    }

    ImGui::PushID("costume");
    _filterCostume.Draw("Filter");
    if (ImGui::ListBoxHeader("Costume")) {
      auto actorKey = actor->getKey();
      std::vector<std::string> entries;
      Locator<EngineSettings>::get().getEntries(entries);
      for (const auto &entry : entries) {
        if (entry.length() < 15)
          continue;
        auto extension = entry.substr(entry.length() - 14, 14);
        CaseInsensitiveCompare cmp;
        if (!cmp(extension, "Animation.json"))
          continue;
        auto prefix = entry.substr(0, actorKey.length());
        if (!cmp(prefix, actorKey))
          continue;
        if (_filterCostume.PassFilter(entry.c_str())) {
          if (ImGui::Selectable(entry.c_str(), actor->getCostume().getPath() == entry)) {
            actor->getCostume().loadCostume(entry);
          }
        }
      }
      ImGui::ListBoxFooter();
    }
    ImGui::PopID();

    if (ImGui::CollapsingHeader("Animations")) {
      ImGui::Indent();

      // actor animations
      auto &anims = actor->getCostume().getAnimations();
      static ImGuiTextFilter filter;
      filter.Draw("Filter");
      if (ImGui::ListBoxHeader("Animations")) {
        for (auto &anim : anims) {
          auto name = anim.getName();
          if (filter.PassFilter(name.c_str())) {
            if (ImGui::Selectable(name.c_str(), _pSelectedAnim == &anim)) {
              _pSelectedAnim = &anim;
            }
          }
        }
        ImGui::ListBoxFooter();
      }
      if (_pSelectedAnim && ImGui::Button("Set")) {
        actor->getCostume().setAnimation(_pSelectedAnim->getName());
      }
      if (_pSelectedAnim) {
        showLayers();
      }
      ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("General")) {
      ImGui::Indent();
      auto isVisible = actor->isVisible();
      if (ImGui::Checkbox("Visible", &isVisible)) {
        actor->setVisible(isVisible);
      }
      auto isTouchable = actor->isTouchable();
      if (ImGui::Checkbox("Touchable", &isTouchable)) {
        actor->setTouchable(isTouchable);
      }
      auto pRoom = actor->getRoom();
      ImGui::Text("Room: %s", pRoom ? pRoom->getName().c_str() : "(none)");
      ImGui::Text("Talking: %s", actor->isTalking() ? "yes" : "no");
      ImGui::Text("Walking: %s", actor->isWalking() ? "yes" : "no");
      auto facing = facingToInt(actor->getCostume().getFacing());
      auto facings = "Front\0Back\0Left\0Right\0";
      if (ImGui::Combo("Facing", &facing, facings)) {
        actor->getCostume().setFacing(intToFacing(facing));
      }
      if (pRoom) {
        auto scale = actor->getScale();
        ImGui::Text("Scale: %.3f", scale);
      }
      auto color = actor->getColor();
      if (ColorEdit4("Color", color)) {
        actor->setColor(color);
      }
      auto talkColor = actor->getTalkColor();
      if (ColorEdit4("Talk color", talkColor)) {
        actor->setTalkColor(talkColor);
      }
      auto pos = actor->getPosition();
      if (InputFloat2("Position", pos)) {
        actor->setPosition(pos);
      }
      auto usePos = actor->getUsePosition().value_or(sf::Vector2f());
      if (InputFloat2("Use Position", usePos)) {
        actor->setUsePosition(usePos);
      }
      auto offset = actor->getOffset();
      if (InputFloat2("Offset", offset)) {
        actor->setOffset(offset);
      }
      auto renderOffset = actor->getRenderOffset();
      if (InputInt2("Render Offset", renderOffset)) {
        actor->setRenderOffset(renderOffset);
      }
      auto walkSpeed = actor->getWalkSpeed();
      if (InputInt2("Walk speed", walkSpeed)) {
        actor->setWalkSpeed(walkSpeed);
      }
      auto hotspotVisible = actor->isHotspotVisible();
      if (ImGui::Checkbox("Show hotspot", &hotspotVisible)) {
        actor->showHotspot(hotspotVisible);
      }
      auto hotspot = actor->getHotspot();
      if (InputInt4("Hotspot", hotspot)) {
        actor->setHotspot(hotspot);
      }
      ImGui::Unindent();
    }
    ImGui::Unindent();
  }

  static int facingToInt(Facing facing) {
    switch (facing) {
    case Facing::FACE_FRONT:return 0;
    case Facing::FACE_BACK:return 1;
    case Facing::FACE_LEFT:return 2;
    case Facing::FACE_RIGHT:return 3;
    }
    return 0;
  }

  static Facing intToFacing(int facing) {
    switch (facing) {
    case 0:return Facing::FACE_FRONT;
    case 1:return Facing::FACE_BACK;
    case 2:return Facing::FACE_LEFT;
    case 3:return Facing::FACE_RIGHT;
    }
    return Facing::FACE_FRONT;
  }

  void showObjects() {
    if (!ImGui::CollapsingHeader("Objects"))
      return;

    auto &objects = _engine.getRoom()->getObjects();

    static ImGuiTextFilter filter;
    filter.Draw("Filter");
    std::ostringstream s;
    s << objects.size() << " Objects";
    if (ImGui::ListBoxHeader(s.str().c_str())) {
      for (const auto &object : objects) {
        auto name = toUtf8(_engine.getText(object->getName()));
        if (filter.PassFilter(name.c_str())) {
          if (ImGui::Selectable(name.c_str(), _pSelectedObject == object.get())) {
            _pSelectedObject = object.get();
          }
        }
      }
      ImGui::ListBoxFooter();
    }

    ImGui::Separator();

    if (!objects.empty() && _pSelectedObject) {
      auto &object = _pSelectedObject;
      static char name[1024] = "";
      strcpy(name, object->getName().c_str());
      if (ImGui::InputText("Name", name, IM_ARRAYSIZE(name))) {
        object->setName(name);
      }
      std::string type;
      switch (object->getType()) {
      case ObjectType::Object:type = "object";
        break;
      case ObjectType::Spot:type = "spot";
        break;
      case ObjectType::Trigger:type = "trigger";
        break;
      case ObjectType::Prop:type = "prop";
        break;
      }
      ImGui::LabelText("Type", "%s", type.c_str());
      auto isVisible = object->isVisible();
      if (ImGui::Checkbox("Visible", &isVisible)) {
        object->setVisible(isVisible);
      }
      auto state = object->getState();
      if (ImGui::InputInt("State", &state)) {
        object->setStateAnimIndex(state);
      }
      auto isTouchable = object->isTouchable();
      if (ImGui::Checkbox("Touchable", &isTouchable)) {
        object->setTouchable(isTouchable);
      }
      auto zorder = object->getZOrder();
      if (ImGui::InputInt("Z-Order", &zorder)) {
        object->setZOrder(zorder);
      }
      auto pos = object->getPosition();
      if (InputFloat2("Position", pos)) {
        object->setPosition(pos);
      }
      auto usePos = object->getUsePosition().value_or(sf::Vector2f());
      if (InputFloat2("Use Position", usePos)) {
        object->setUsePosition(usePos);
      }
      auto offset = object->getOffset();
      if (InputFloat2("Offset", offset)) {
        object->setOffset(offset);
      }
      auto renderOffset = object->getRenderOffset();
      if (InputInt2("Render Offset", renderOffset)) {
        object->setRenderOffset(renderOffset);
      }
      auto hotspotVisible = object->isHotspotVisible();
      if (ImGui::Checkbox("Show hotspot", &hotspotVisible)) {
        object->showDebugHotspot(hotspotVisible);
      }
      auto hotspot = object->getHotspot();
      if (InputInt4("Hotspot", hotspot)) {
        object->setHotspot(hotspot);
      }
      auto color = object->getColor();
      if (ColorEdit4("Color", color)) {
        object->setColor(color);
      }
      auto trigger = object->getTrigger();
      if (trigger) {
        ImGui::LabelText("Trigger", "%s", trigger->getName().c_str());
      }
    }
  }

  void showRooms() {
    if (!ImGui::CollapsingHeader("Room"))
      return;

    auto &rooms = _engine.getRooms();
    _roomInfos.clear();
    int i = 0;
    int currentRoom = 0;
    for (auto &&room : rooms) {
      if (room.get() == _engine.getRoom()) {
        currentRoom = i;
      }
      _roomInfos.push_back(room->getName());
      i++;
    }

    if (ImGui::Combo("Room", &currentRoom, stringGetter, static_cast<void *>(&_roomInfos), rooms.size())) {
      _engine.setRoom(rooms[currentRoom].get());
    }
    auto &room = rooms[currentRoom];

    auto options = _engine.getWalkboxesFlags();
    auto showWalkboxes = (options & 4) != 0;
    if (ImGui::Checkbox("Walkboxes", &showWalkboxes)) {
      _engine.setWalkboxesFlags(showWalkboxes ? (4 | options) : (options & ~4));
    }
    auto showMergedWalkboxes = (options & 1) != 0;
    if (ImGui::Checkbox("Merged Walkboxes", &showMergedWalkboxes)) {
      _engine.setWalkboxesFlags(showMergedWalkboxes ? (1 | options) : (options & ~1));
    }
    auto showGraph = (options & 2) != 0;
    if (ImGui::Checkbox("Graph", &showGraph)) {
      _engine.setWalkboxesFlags(showGraph ? (2 | options) : (options & ~2));
    }
    updateWalkboxInfos(room.get());
    ImGui::Combo("##walkboxes", &_selectedWalkbox, stringGetter, static_cast<void *>(&_walkboxInfos),
                 _walkboxInfos.size());
    auto rotation = room->getRotation();
    if (ImGui::SliderFloat("rotation", &rotation, -180.f, 180.f, "%.0f deg")) {
      room->setRotation(rotation);
    }
    auto overlay = room->getOverlayColor();
    if (ColorEdit4("Overlay", overlay)) {
      room->setOverlayColor(overlay);
    }
    auto ambient = room->getAmbientLight();
    if (ColorEdit4("ambient", ambient)) {
      room->setAmbientLight(ambient);
    }
    auto effect = room->getEffect();
    auto effects = "None\0Sepia\0EGA\0VHS\0Ghost\0Black & White\0";
    if (ImGui::Combo("Shader", &effect, effects)) {
      room->setEffect(effect);
    }
  }

  void updateWalkboxInfos(Room *pRoom) {
    _walkboxInfos.clear();
    if (!pRoom)
      return;
    auto &walkboxes = pRoom->getWalkboxes();
    for (size_t i = 0; i < walkboxes.size(); ++i) {
      auto walkbox = walkboxes.at(i);
      auto name = walkbox.getName();
      std::ostringstream s;
      if (!name.empty()) {
        s << name;
      } else {
        s << "Walkbox #" << i;
      }
      s << " " << (walkbox.isEnabled() ? "[enabled]" : "[disabled]");
      _walkboxInfos.push_back(s.str());
    }
  }

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

private:
  Engine &_engine;
  int _selectedActor{0};
  Object *_pSelectedObject{nullptr};
  int _selectedStack{0};
  int _selectedWalkbox{0};
  std::vector<std::string> _walkboxInfos;
  std::vector<std::string> _actorInfos;
  std::vector<std::string> _roomInfos;
  static const char *_langs[];
  CostumeAnimation *_pSelectedAnim{nullptr};
  ImGuiTextFilter _filterCostume;
};
const char *_DebugTools::_langs[] = {"en", "fr", "de", "es", "it"};
} // namespace ng
