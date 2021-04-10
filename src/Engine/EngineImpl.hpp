#include <squirrel.h>
#include <glm/gtc/epsilon.hpp>
#include <engge/Engine/Engine.hpp>
#include <engge/Engine/ActorIconSlot.hpp>
#include <engge/Engine/ActorIcons.hpp>
#include <engge/Engine/Camera.hpp>
#include <engge/Engine/Cutscene.hpp>
#include <engge/Engine/Hud.hpp>
#include <engge/Input/InputConstants.hpp>
#include <engge/Dialog/DialogManager.hpp>
#include <engge/Graphics/GGFont.hpp>
#include <engge/Engine/Inventory.hpp>
#include <engge/UI/OptionsDialog.hpp>
#include <engge/UI/StartScreenDialog.hpp>
#include <engge/Engine/Preferences.hpp>
#include <engge/Room/Room.hpp>
#include <engge/Room/RoomScaling.hpp>
#include <engge/Graphics/Screen.hpp>
#include <engge/Scripting/ScriptEngine.hpp>
#include <engge/Scripting/ScriptExecute.hpp>
#include <engge/Engine/Sentence.hpp>
#include <engge/Audio/SoundDefinition.hpp>
#include <engge/Audio/SoundManager.hpp>
#include <engge/Graphics/SpriteSheet.hpp>
#include <engge/Engine/TextDatabase.hpp>
#include <engge/Engine/Thread.hpp>
#include <engge/Engine/Verb.hpp>
#include <engge/Scripting/VerbExecute.hpp>
#include <squirrel.h>
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
#include <cmath>
#include <ctime>
#include <cctype>
#include <cwchar>
#include <filesystem>
#include <iomanip>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <ngf/Graphics/Sprite.h>
#include <ngf/Graphics/RenderTexture.h>
#include <ngf/Graphics/RectangleShape.h>
#include <ngf/Graphics/Text.h>
#include <imgui.h>
#include <engge/Engine/EngineSettings.hpp>
#include <engge/Input/CommandManager.hpp>
#include <engge/Engine/EngineCommands.hpp>
#include <engge/System/Logger.hpp>
#include <engge/Parsers/SavegameManager.hpp>
#include "DebugFeatures.hpp"
#include "Entities/TalkingState.hpp"
#include "Graphics/WalkboxDrawable.hpp"
#include "Graphics/GraphDrawable.hpp"
#include "Shaders.hpp"
namespace fs = std::filesystem;

namespace ng {
static const char *const objectKey = "_objectKey";
static const char *const roomKey = "_roomKey";
static const char *const actorKey = "_actorKey";
static const char *const idKey = "_id";
static const char *const pseudoObjectsKey = "_pseudoObjects";
static const auto clickedAtCallback = "clickedAt";

enum class CursorDirection : unsigned int {
  None = 0,
  Left = 1,
  Right = 1u << 1u,
  Up = 1u << 2u,
  Down = 1u << 3u,
  Hotspot = 1u << 4u
};

enum class EngineState {
  Game, Paused, Options, StartScreen, Quit
};

struct Engine::Impl {
  class SaveGameSystem {
  public:
    explicit SaveGameSystem(Engine::Impl *pImpl) : m_pImpl(pImpl) {}

    void saveGame(const std::filesystem::path &path) {
      ScriptEngine::call("preSave");

      time_t now;
      time(&now);

      ngf::StopWatch watch;

      SQObjectPtr g;
      _table(ScriptEngine::getVm()->_roottable)->Get(ScriptEngine::toSquirrel("g"), g);
      SQObjectPtr easyMode;
      _table(g)->Get(ScriptEngine::toSquirrel("easy_mode"), easyMode);

      ngf::GGPackValue saveGameHash = {
          {"actors", saveActors()},
          {"callbacks", saveCallbacks()},
          {"currentRoom", m_pImpl->m_pRoom->getName()},
          {"dialog", saveDialogs()},
          {"easy_mode", static_cast<int>(_integer(easyMode))},
          {"gameGUID", std::string()},
          {"gameScene", saveGameScene()},
          {"gameTime", m_pImpl->m_time.getTotalSeconds()},
          {"globals", saveGlobals()},
          {"inputState", m_pImpl->m_pEngine->getInputState()},
          {"inventory", saveInventory()},
          {"objects", saveObjects()},
          {"rooms", saveRooms()},
          {"savebuild", 958},
          {"savetime", static_cast<int>(now)},
          {"selectedActor", m_pImpl->m_pEngine->getCurrentActor()->getKey()},
          {"version", 2},
      };

      SavegameManager::saveGame(path, saveGameHash);

      info("Save game in {} s", watch.getElapsedTime().getTotalSeconds());

      ScriptEngine::call("postSave");
    }

    void loadGame(const std::string &path) {
      auto hash = SavegameManager::loadGame(path);

      std::ofstream os(path + ".json");
      os << hash;
      os.close();

      loadGame(hash);
    }

    static std::filesystem::path getSlotPath(int slot) {
      std::ostringstream filename;
      filename << "Savegame" << slot << ".save";
      auto path = Locator<EngineSettings>::get().getPath();
      path.append(filename.str());
      return path;
    }

    static void getSlot(SavegameSlot &slot) {
      auto hash = SavegameManager::loadGame(slot.path);
      slot.easyMode = hash["easy_mode"].getInt() != 0;
      slot.savetime = (time_t) hash["savetime"].getInt();
      slot.gametime = ngf::TimeSpan::seconds(static_cast<float>(hash["gameTime"].getDouble()));
    }

  private:
    static std::string getValue(const ngf::GGPackValue &property) {
      std::ostringstream s;
      if (property.isInteger()) {
        s << property.getInt();
      } else if (property.isDouble()) {
        s << property.getDouble();
      } else if (property.isString()) {
        s << property.getString();
      }
      return s.str();
    }

    SQObjectPtr toSquirrel(const ngf::GGPackValue &value) {
      if (value.isString()) {
        return ScriptEngine::toSquirrel(value.getString());
      }
      if (value.isInteger()) {
        return static_cast<SQInteger>(value.getInt());
      }
      if (value.isDouble()) {
        return static_cast<SQFloat>(value.getDouble());
      }
      if (value.isArray()) {
        auto array = SQArray::Create(_ss(ScriptEngine::getVm()), value.size());
        SQInteger i = 0;
        for (auto &item : value) {
          array->Set(i++, toSquirrel(item));
        }
        return array;
      }
      if (value.isHash()) {
        auto actor = value[actorKey];
        if (!actor.isNull()) {
          auto pActor = getActor(actor.getString());
          return pActor->getTable();
        }
        auto object = value[objectKey];
        auto room = value[roomKey];
        if (!object.isNull()) {
          Object *pObject;
          if (!room.isNull()) {
            auto pRoom = getRoom(room.getString());
            pObject = getObject(pRoom, object.getString());
            if (!pObject) {
              warn("load: object {} not found", object.getString());
              return SQObjectPtr();
            }
            return pObject->getTable();
          }
          pObject = getObject(object.getString());
          if (!pObject) {
            warn("load: object {} not found", object.getString());
            return SQObjectPtr();
          }
          return pObject->getTable();
        }

        if (!room.isNull()) {
          auto pRoom = getRoom(room.getString());
          return pRoom->getTable();
        }

        auto table = SQTable::Create(_ss(ScriptEngine::getVm()), 0);
        for (const auto&[key, value] : value.items()) {
          table->NewSlot(ScriptEngine::toSquirrel(key), toSquirrel(value));
        }
        return table;
      }
      if (!value.isNull()) {
        warn("trying to convert an unknown value (type={}) to squirrel", static_cast<int >(value.type()));
      }
      return SQObjectPtr();
    }

    void loadGameScene(const ngf::GGPackValue &hash) {
      auto actorsSelectable = hash["actorsSelectable"].getInt();
      auto actorsTempUnselectable = hash["actorsTempUnselectable"].getInt();
      auto mode = actorsSelectable ? ActorSlotSelectableMode::On : ActorSlotSelectableMode::Off;
      if (actorsTempUnselectable) {
        mode |= ActorSlotSelectableMode::TemporaryUnselectable;
      }
      m_pImpl->m_pEngine->setActorSlotSelectable(mode);
      auto forceTalkieText = hash["forceTalkieText"].getInt() != 0;
      m_pImpl->m_pEngine->getPreferences().setTempPreference(TempPreferenceNames::ForceTalkieText, forceTalkieText);
      for (const auto &selectableActor : hash["selectableActors"]) {
        auto pActor = getActor(selectableActor[actorKey].getString());
        auto selectable = selectableActor["selectable"].getInt() != 0;
        m_pImpl->m_pEngine->actorSlotSelectable(pActor, selectable);
      }
    }

    void loadDialog(const ngf::GGPackValue &hash) {
      auto &states = m_pImpl->m_dialogManager.getStates();
      states.clear();
      for (auto &property : hash.items()) {
        const auto &dialog = property.key();
        // dialog format: mode dialog number actor
        // example: #ChetAgentStreetDialog14reyes
        // mode:
        // ?: once
        // #: showonce
        // &: onceever
        // $: showonceever
        // ^: temponce
        auto state = parseState(dialog);
        states.push_back(state);
        // TODO: what to do with this dialog value ?
        //auto value = property.second.getInt();
      }
    }

    [[nodiscard]] static DialogConditionState parseState(const std::string &dialog) {
      DialogConditionState state;
      switch (dialog[0]) {
      case '?':state.mode = DialogConditionMode::Once;
        break;
      case '#':state.mode = DialogConditionMode::ShowOnce;
        break;
      case '&':state.mode = DialogConditionMode::OnceEver;
        break;
      case '$':state.mode = DialogConditionMode::ShowOnceEver;
        break;
      case '^':state.mode = DialogConditionMode::TempOnce;
        break;
      }
      std::string dialogName;
      int i;
      for (i = 1; i < static_cast<int>(dialog.length()) && !isdigit(dialog[i]); i++) {
        dialogName.append(1, dialog[i]);
      }
      auto &settings = Locator<EngineSettings>::get();
      while (!settings.hasEntry(dialogName + ".byack")) {
        dialogName.append(1, dialog.at(i++));
      }
      std::string num;
      state.dialog = dialogName;
      for (; i < static_cast<int>(dialog.length()) && isdigit(dialog[i]); i++) {
        num.append(1, dialog[i]);
      }
      state.line = atol(num.data());
      state.actorKey = dialog.substr(i);
      return state;
    }

    void loadCallbacks(const ngf::GGPackValue &hash) {
      m_pImpl->m_callbacks.clear();
      for (auto &callBackHash : hash["callbacks"]) {
        auto name = callBackHash["function"].getString();
        auto id = callBackHash["guid"].getInt();
        auto time = ngf::TimeSpan::seconds(static_cast<float>(callBackHash["time"].getInt()) / 1000.f);
        auto arg = toSquirrel(callBackHash["param"]);
        auto callback = std::make_unique<Callback>(id, time, name, arg);
        m_pImpl->m_callbacks.push_back(std::move(callback));
      }
      Locator<EntityManager>::get().setCallbackId(hash["nextGuid"].getInt());
    }

    void loadActors(const ngf::GGPackValue &hash) {
      for (auto &pActor : m_pImpl->m_actors) {
        if (pActor->getKey().empty())
          continue;

        const auto &actorHash = hash[pActor->getKey()];
        loadActor(pActor.get(), actorHash);
      }
    }

    void loadActor(Actor *pActor, const ngf::GGPackValue &actorHash) {
      glm::vec2 pos{0, 0};
      getValue(actorHash, "_pos", pos);
      pActor->setPosition(pos);

      std::string costume;
      getValue(actorHash, "_costume", costume);
      std::string costumesheet;
      getValue(actorHash, "_costumeSheet", costumesheet);
      pActor->getCostume().loadCostume(costume, costumesheet);

      std::string room;
      getValue(actorHash, roomKey, room);
      auto *pRoom = getRoom(room.empty() ? "Void" : room);
      pActor->setRoom(pRoom);

      int dir = static_cast<int>(Facing::FACE_FRONT);
      getValue(actorHash, "_dir", dir);
      pActor->getCostume().setFacing(static_cast<Facing>(dir));

      int useDirValue = static_cast<int>(UseDirection::Front);
      getValue(actorHash, "_useDir", useDirValue);
      pActor->setUseDirection(static_cast<UseDirection>(dir));

      int lockFacing = 0;
      getValue(actorHash, "_lockFacing", lockFacing);
      if (lockFacing == 0) {
        pActor->getCostume().unlockFacing();
      } else {
        pActor->getCostume().lockFacing(static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing),
                                        static_cast<Facing>(lockFacing));
      }

      glm::vec2 usePos{0, 0};
      getValue(actorHash, "_usePos", usePos);
      pActor->setUsePosition(usePos);

      glm::vec2 renderOffset{0, 45};
      getValue(actorHash, "_renderOffset", renderOffset);
      pActor->setRenderOffset((glm::ivec2) renderOffset);

      glm::vec2 offset{0, 0};
      getValue(actorHash, "_offset", offset);
      pActor->setOffset(offset);

      for (auto &property : actorHash.items()) {
        if (property.key().empty() || property.key()[0] == '_') {
          if (property.key() == "_animations") {
            std::vector<std::string> anims;
            for (auto &value : property.value()) {
              anims.push_back(value.getString());
            }
            // TODO: _animations
            trace("load: actor {} property '{}' not loaded (type={}) size={}",
                  pActor->getKey(),
                  property.key(),
                  static_cast<int>(property.value().type()), anims.size());
          } else if ((property.key() == "_pos")
              || (property.key() == "_costume")
              || (property.key() == "_costumeSheet")
              || (property.key() == roomKey)
              || (property.key() == "_dir")
              || (property.key() == "_useDir")
              || (property.key() == "_lockFacing")
              || (property.key() == "_usePos")
              || (property.key() == "_renderOffset")
              || (property.key() == "_offset")) {
          } else {
            _table(pActor->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
          }
          continue;
        }

        _table(pActor->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
      }
      if (ScriptEngine::rawExists(pActor, "postLoad")) {
        ScriptEngine::objCall(pActor, "postLoad");
      }
    }

    void loadInventory(const ngf::GGPackValue &hash) {
      for (auto i = 0; i < static_cast<int>(m_pImpl->m_actorsIconSlots.size()); ++i) {
        auto *pActor = m_pImpl->m_actorsIconSlots[i].pActor;
        if (!pActor)
          continue;
        auto &slot = hash["slots"].at(i);
        pActor->clearInventory();
        int jiggleCount = 0;
        for (const auto &obj : slot["objects"]) {
          auto pObj = getInventoryObject(obj.getString());
          // TODO: why we don't find the inventory object here ?
          if (!pObj)
            continue;
          const auto jiggle = slot["jiggle"].isArray() && slot["jiggle"][jiggleCount++].getInt() != 0;
          pObj->setJiggle(jiggle);
          pActor->pickupObject(pObj);
        }
        auto scroll = slot["scroll"].getInt();
        pActor->setInventoryOffset(scroll);
      }
    }

    void loadObjects(const ngf::GGPackValue &hash) {
      for (auto &obj :  hash.items()) {
        const auto &objName = obj.key();
        if (objName.empty())
          continue;
        auto pObj = getObject(objName);
        // TODO: if the object does not exist creates it
        if (!pObj) {
          trace("load: object '{}' not loaded because it has not been found", objName);
          continue;
        }
        loadObject(pObj, obj.value());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, int &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getInt();
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, std::string &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getString();
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, float &value) {
      if (!hash[key].isNull()) {
        value = static_cast<float>(hash[key].getDouble());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, bool &value) {
      if (!hash[key].isNull()) {
        value = hash[key].getInt() != 0;
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, glm::vec2 &value) {
      if (!hash[key].isNull()) {
        value = parsePos(hash[key].getString());
      }
    }

    static void getValue(const ngf::GGPackValue &hash, const std::string &key, ngf::Color &value) {
      if (!hash[key].isNull()) {
        value = fromRgba(hash[key].getInt());
      }
    }

    void loadObject(Object *pObj, const ngf::GGPackValue &hash) {
      auto state = 0;
      getValue(hash, "_state", state);
      pObj->setStateAnimIndex(state);
      glm::vec2 offset{0, 0};
      getValue(hash, "_offset", offset);
      pObj->setOffset(offset);
      float rotation = 0;
      getValue(hash, "_rotation", rotation);
      pObj->setRotation(rotation);

      for (auto &property :  hash.items()) {
        if (property.key().empty() || property.key()[0] == '_') {
          if (property.key() == "_state" || property.key() == "_offset" || property.key() == "_rotation")
            continue;

          _table(pObj->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
          continue;
        }

        _table(pObj->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
      }
    }

    void loadPseudoObjects(Room *pRoom, const ngf::GGPackValue &hash) {
      for (const auto &entry :  hash.items()) {
        auto pObj = getObject(pRoom, entry.key());
        if (!pObj) {
          trace("load: room '{}' object '{}' not loaded because it has not been found", pRoom->getName(), entry.key());
          continue;
        }
        loadObject(pObj, entry.value());
      }
    }

    void loadRooms(const ngf::GGPackValue &hash) {
      for (auto &roomHash :  hash.items()) {
        const auto &roomName = roomHash.key();
        auto pRoom = getRoom(roomName);
        if (!pRoom) {
          trace("load: room '{}' not loaded because it has not been found", roomName);
          continue;
        }

        for (auto &property : roomHash.value().items()) {
          if (property.key().empty() || property.key()[0] == '_') {
            if (property.key() == pseudoObjectsKey) {
              loadPseudoObjects(pRoom, property.value());
            } else {
              trace("load: room '{}' property '{}' (type={}) not loaded",
                    roomName,
                    property.key(),
                    static_cast<int>(property.value().type()));
              continue;
            }
          }

          _table(pRoom->getTable())->Set(ScriptEngine::toSquirrel(property.key()), toSquirrel(property.value()));
          if (ScriptEngine::rawExists(pRoom, "postLoad")) {
            ScriptEngine::objCall(pRoom, "postLoad");
          }
        }
      }
    }

    void loadGame(const ngf::GGPackValue &hash) {
      auto version = hash["version"].getInt();
      if (version != 2) {
        warn("Cannot load savegame version {}", version);
        return;
      }

      ScriptEngine::call("preLoad");

      loadGameScene(hash["gameScene"]);
      loadDialog(hash["dialog"]);
      loadCallbacks(hash["callbacks"]);
      loadGlobals(hash["globals"]);
      loadActors(hash["actors"]);
      loadInventory(hash["inventory"]);
      loadRooms(hash["rooms"]);

      m_pImpl->m_time = ngf::TimeSpan::seconds(static_cast<float>(hash["gameTime"].getDouble()));
      m_pImpl->m_pEngine->setInputState(hash["inputState"].getInt());

      loadObjects(hash["objects"]);
      setActor(hash["selectedActor"].getString());
      setCurrentRoom(hash["currentRoom"].getString());

      ScriptEngine::set("SAVEBUILD", hash["savebuild"].getInt());

      ScriptEngine::call("postLoad");
    }

    void setActor(const std::string &name) {
      auto *pActor = getActor(name);
      m_pImpl->m_pEngine->setCurrentActor(pActor, false);
    }

    Actor *getActor(const std::string &name) {
      return dynamic_cast<Actor *>(m_pImpl->m_pEngine->getEntity(name));
    }

    Room *getRoom(const std::string &name) {
      auto &rooms = m_pImpl->m_pEngine->getRooms();
      auto it = std::find_if(rooms.begin(), rooms.end(), [&name](auto &pRoom) { return pRoom->getName() == name; });
      if (it != rooms.end())
        return it->get();
      return nullptr;
    }

    static Object *getInventoryObject(const std::string &name) {
      auto v = ScriptEngine::getVm();
      SQObjectPtr obj;
      if (!_table(v->_roottable)->Get(ScriptEngine::toSquirrel(name), obj)) {
        return nullptr;
      }
      SQObjectPtr id;
      if (!_table(obj)->Get(ScriptEngine::toSquirrel(idKey), id)) {
        return nullptr;
      }
      return EntityManager::getObjectFromId(static_cast<int>(_integer(id)));
    }

    Object *getObject(const std::string &name) {
      for (auto &pRoom : m_pImpl->m_rooms) {
        for (auto &pObj : pRoom->getObjects()) {
          if (pObj->getKey() == name)
            return pObj.get();
        }
      }
      return nullptr;
    }

    static Object *getObject(Room *pRoom, const std::string &name) {
      for (auto &pObj : pRoom->getObjects()) {
        if (pObj->getKey() == name)
          return pObj.get();
      }
      return nullptr;
    }

    void setCurrentRoom(const std::string &name) {
      m_pImpl->m_pEngine->setRoom(getRoom(name));
    }

    [[nodiscard]] ngf::GGPackValue saveActors() const {
      ngf::GGPackValue actorsHash;
      for (auto &pActor : m_pImpl->m_actors) {
        // TODO: find why this entry exists...
        if (pActor->getKey().empty())
          continue;

        auto table = pActor->getTable();
        auto actorHash = ng::toGGPackValue(table);
        auto costume = fs::path(pActor->getCostume().getPath()).filename();
        if (costume.has_extension())
          costume.replace_extension();
        actorHash["_costume"] = costume.u8string();
        actorHash["_dir"] = static_cast<int>(pActor->getCostume().getFacing());
        auto lockFacing = pActor->getCostume().getLockFacing();
        actorHash["_lockFacing"] = lockFacing.has_value() ? static_cast<int>(lockFacing.value()) : 0;
        actorHash["_pos"] = toString(pActor->getPosition());
        auto useDir = pActor->getUseDirection();
        if (useDir.has_value()) {
          actorHash["_useDir"] = static_cast<int>(useDir.value());
        }
        auto usePos = pActor->getUsePosition();
        if (useDir.has_value()) {
          actorHash["_usePos"] = toString(usePos.value());
        }
        auto renderOffset = pActor->getRenderOffset();
        if (renderOffset != glm::ivec2(0, 45)) {
          actorHash["_renderOffset"] = toString(renderOffset);
        }
        auto costumeSheet = pActor->getCostume().getSheet();
        if (!costumeSheet.empty()) {
          actorHash["_costumeSheet"] = costumeSheet;
        }
        if (pActor->getRoom()) {
          actorHash[roomKey] = pActor->getRoom()->getName();
        } else {
          actorHash[roomKey] = nullptr;
        }

        actorsHash[pActor->getKey()] = actorHash;
      }
      return actorsHash;
    }

    [[nodiscard]] static ngf::GGPackValue saveGlobals() {
      auto v = ScriptEngine::getVm();
      auto top = sq_gettop(v);
      sq_pushroottable(v);
      sq_pushstring(v, _SC("g"), -1);
      sq_get(v, -2);
      HSQOBJECT g;
      sq_getstackobj(v, -1, &g);

      auto globalsHash = ng::toGGPackValue(g);
      sq_settop(v, top);
      return globalsHash;
    }

    [[nodiscard]] ngf::GGPackValue saveDialogs() const {
      ngf::GGPackValue hash;
      const auto &states = m_pImpl->m_dialogManager.getStates();
      for (const auto &state : states) {
        std::ostringstream s;
        switch (state.mode) {
        case DialogConditionMode::TempOnce:continue;
        case DialogConditionMode::OnceEver:s << "&";
          break;
        case DialogConditionMode::ShowOnce:s << "#";
          break;
        case DialogConditionMode::Once:s << "?";
          break;
        case DialogConditionMode::ShowOnceEver:s << "$";
          break;
        }
        s << state.dialog << state.line << state.actorKey;
        // TODO: value should be 1 or another value ?
        hash[s.str()] = state.mode == DialogConditionMode::ShowOnce ? 2 : 1;
      }
      return hash;
    }

    [[nodiscard]] ngf::GGPackValue saveGameScene() const {
      auto actorsSelectable =
          ((m_pImpl->m_actorIcons.getMode() & ActorSlotSelectableMode::On) == ActorSlotSelectableMode::On);
      auto actorsTempUnselectable = ((m_pImpl->m_actorIcons.getMode() & ActorSlotSelectableMode::TemporaryUnselectable)
          == ActorSlotSelectableMode::TemporaryUnselectable);

      ngf::GGPackValue selectableActors;
      for (auto &slot : m_pImpl->m_actorsIconSlots) {
        ngf::GGPackValue selectableActor;
        if (slot.pActor) {
          selectableActor = {
              {actorKey, slot.pActor->getKey()},
              {"selectable", slot.selectable ? 1 : 0},
          };
        } else {
          selectableActor = {{"selectable", 0}};
        }
        selectableActors.push_back(selectableActor);
      }

      auto forceTalkieText = m_pImpl->m_pEngine->getPreferences()
          .getTempPreference(TempPreferenceNames::ForceTalkieText,
                             TempPreferenceDefaultValues::ForceTalkieText);
      return {
          {"actorsSelectable", actorsSelectable ? 1 : 0},
          {"actorsTempUnselectable", actorsTempUnselectable ? 1 : 0},
          {"forceTalkieText", forceTalkieText ? 1 : 0},
          {"selectableActors", selectableActors}
      };
    }

    [[nodiscard]] ngf::GGPackValue saveInventory() const {
      ngf::GGPackValue slots;
      for (auto &slot : m_pImpl->m_actorsIconSlots) {
        ngf::GGPackValue actorSlot;
        if (slot.pActor) {
          std::vector<int> jiggleArray(slot.pActor->getObjects().size());
          ngf::GGPackValue objects;
          int jiggleCount = 0;
          for (auto &obj : slot.pActor->getObjects()) {
            jiggleArray[jiggleCount++] = obj->getJiggle() ? 1 : 0;
            objects.push_back(obj->getKey());
          }
          actorSlot = {
              {"objects", objects},
              {"scroll", slot.pActor->getInventoryOffset()},
          };
          const auto saveJiggle = std::any_of(jiggleArray.cbegin(),
                                              jiggleArray.cend(), [](int value) { return value == 1; });
          if (saveJiggle) {
            ngf::GGPackValue jiggle;
            std::copy(jiggleArray.cbegin(),
                      jiggleArray.cend(),
                      std::back_inserter(jiggle));
            actorSlot["jiggle"] = jiggle;
          }
        } else {
          actorSlot = {{"scroll", 0}};
        }
        slots.push_back(actorSlot);
      }

      return {
          {"slots", slots},
      };
    }

    [[nodiscard]] ngf::GGPackValue saveObjects() const {
      ngf::GGPackValue hash;
      for (auto &room : m_pImpl->m_rooms) {
        for (auto &object : room->getObjects()) {
          if (object->getType() != ObjectType::Object)
            continue;
          auto pRoom = object->getRoom();
          if (pRoom && pRoom->isPseudoRoom())
            continue;
          hash[object->getKey()] = saveObject(object.get());
        }
      }
      return hash;
    }

    static ngf::GGPackValue savePseudoObjects(const Room *pRoom) {
      ngf::GGPackValue hashObjects;
      for (const auto &pObj : pRoom->getObjects()) {
        hashObjects[pObj->getKey()] = saveObject(pObj.get());
      }
      return hashObjects;
    }

    static ngf::GGPackValue saveObject(const Object *pObject) {
      auto hashObject = ng::toGGPackValue(pObject->getTable());
      if (pObject->getState() != 0) {
        hashObject["_state"] = pObject->getState();
      }
      if (!pObject->isTouchable()) {
        hashObject["_touchable"] = 0;
      }
      // this is the way to compare 2 vectors... not so simple
      if (glm::any(glm::epsilonNotEqual(pObject->getOffset(), glm::vec2(0, 0), 1e-6f))) {
        hashObject["_offset"] = toString(pObject->getOffset());
      }
      return hashObject;
    }

    [[nodiscard]] ngf::GGPackValue saveRooms() const {
      ngf::GGPackValue hash;
      for (auto &room : m_pImpl->m_rooms) {
        auto hashRoom = ng::toGGPackValue(room->getTable());
        if (room->isPseudoRoom()) {
          hashRoom[pseudoObjectsKey] = savePseudoObjects(room.get());
        }
        hash[room->getName()] = hashRoom;
      }
      return hash;
    }

    [[nodiscard]] ngf::GGPackValue saveCallbacks() const {
      ngf::GGPackValue callbacksArray;
      for (auto &callback : m_pImpl->m_callbacks) {
        ngf::GGPackValue callbackHash{
            {"function", callback->getMethod()},
            {"guid", callback->getId()},
            {"time", callback->getElapsed().getTotalMilliseconds()}
        };
        auto arg = callback->getArgument();
        if (arg._type != OT_NULL) {
          callbackHash["param"] = ng::toGGPackValue(arg);
        }
        callbacksArray.push_back(callbackHash);
      }

      auto &resourceManager = Locator<EntityManager>::get();
      auto id = resourceManager.getCallbackId();
      resourceManager.setCallbackId(id);

      return {
          {"callbacks", callbacksArray},
          {"nextGuid", id},
      };
    }

    void loadGlobals(const ngf::GGPackValue &hash) {
      SQTable *pRootTable = _table(ScriptEngine::getVm()->_roottable);
      SQObjectPtr gObject;
      pRootTable->Get(ScriptEngine::toSquirrel("g"), gObject);
      SQTable *gTable = _table(gObject);
      for (const auto &variable : hash.items()) {
        gTable->Set(ScriptEngine::toSquirrel(variable.key()), toSquirrel(variable.value()));
      }
    }

    static std::string toString(const glm::vec2 &pos) {
      std::ostringstream os;
      os << "{" << static_cast<int>(pos.x) << "," << static_cast<int>(pos.y) << "}";
      return os.str();
    }

    static std::string toString(const glm::ivec2 &pos) {
      std::ostringstream os;
      os << "{" << pos.x << "," << pos.y << "}";
      return os.str();
    }

  private:
    Impl *m_pImpl{nullptr};
  };

  Engine *m_pEngine{nullptr};
  ResourceManager &m_resourceManager;
  Room *m_pRoom{nullptr};
  int m_roomEffect{0};
  ngf::Shader m_roomShader;
  ngf::Shader m_fadeShader;
  ngf::Texture m_blackTexture;
  std::vector<std::unique_ptr<Actor>> m_actors;
  std::vector<std::unique_ptr<Room>> m_rooms;
  std::vector<std::unique_ptr<Function>> m_newFunctions;
  std::vector<std::unique_ptr<Function>> m_functions;
  std::vector<std::unique_ptr<Callback>> m_callbacks;
  Cutscene *m_pCutscene{nullptr};
  ng::EnggeApplication *m_pApp{nullptr};
  Actor *m_pCurrentActor{nullptr};
  bool m_inputHUD{false};
  bool m_inputActive{false};
  bool m_showCursor{true};
  bool m_inputVerbsActive{false};
  Actor *m_pFollowActor{nullptr};
  Entity *m_pUseObject{nullptr};
  int m_objId1{0};
  Entity *m_pObj2{nullptr};
  glm::vec2 m_mousePos{0, 0};
  glm::vec2 m_mousePosInRoom{0, 0};
  std::unique_ptr<VerbExecute> m_pVerbExecute;
  std::unique_ptr<ScriptExecute> m_pScriptExecute;
  std::vector<std::unique_ptr<ThreadBase>> m_threads;
  DialogManager m_dialogManager;
  Preferences &m_preferences;
  SoundManager &m_soundManager;
  CursorDirection m_cursorDirection{CursorDirection::None};
  std::array<ActorIconSlot, 6> m_actorsIconSlots;
  UseFlag m_useFlag{UseFlag::None};
  ActorIcons m_actorIcons;
  ngf::TimeSpan m_time;
  bool m_isMouseDown{false};
  ngf::TimeSpan m_mouseDownTime;
  bool m_isMouseRightDown{false};
  int m_frameCounter{0};
  HSQOBJECT m_pDefaultObject{};
  Camera m_camera;
  std::unique_ptr<Sentence> m_pSentence{};
  std::unordered_set<Input, InputHash> m_oldKeyDowns;
  std::unordered_set<Input, InputHash> m_newKeyDowns;
  EngineState m_state{EngineState::StartScreen};
  TalkingState m_talkingState;
  WalkboxesFlags m_showDrawWalkboxes{WalkboxesFlags::None};
  OptionsDialog m_optionsDialog;
  StartScreenDialog m_startScreenDialog;
  bool m_run{false};
  ngf::TimeSpan m_noOverrideElapsed{ngf::TimeSpan::seconds(2)};
  Hud m_hud;
  bool m_autoSave{true};
  bool m_cursorVisible{true};
  FadeEffectParameters m_fadeEffect;

  Impl();

  void drawHud(ngf::RenderTarget &target) const;
  void drawCursor(ngf::RenderTarget &target) const;
  void drawCursorText(ngf::RenderTarget &target) const;
  void drawNoOverride(ngf::RenderTarget &target) const;
  void drawActorHotspot(ngf::RenderTarget &target) const;
  void drawObjectHotspot(const Object &obj, ngf::RenderTarget &target) const;
  void drawDebugHotspot(const Object &object, ngf::RenderTarget &target) const;
  static void drawScreenSpace(const Object &object, ngf::RenderTarget &target, ngf::RenderStates states);
  glm::vec2 roomToScreen(const glm::vec2 &pos) const;
  ngf::irect roomToScreen(const ngf::irect &rect) const;
  int getCurrentActorIndex() const;
  ngf::irect getCursorRect() const;
  void appendUseFlag(std::wstring &sentence) const;
  bool clickedAt(const glm::vec2 &pos) const;
  void updateCutscene(const ngf::TimeSpan &elapsed);
  void updateFunctions(const ngf::TimeSpan &elapsed);
  void updateActorIcons(const ngf::TimeSpan &elapsed);
  void updateSentence(const ngf::TimeSpan &elapsed) const;
  void updateMouseCursor();
  void updateHoveredEntity(bool isRightClick);
  SQInteger enterRoom(Room *pRoom, Object *pObject) const;
  SQInteger exitRoom(Object *pObject);
  void updateRoomScalings() const;
  void setCurrentRoom(Room *pRoom);
  uint32_t getFlags(int id) const;
  uint32_t getFlags(Entity *pEntity) const;
  Entity *getHoveredEntity(const glm::vec2 &mousPos);
  void actorEnter() const;
  void actorExit() const;
  static void onLanguageChange(const std::string &lang);
  void onVerbClick(const Verb *pVerb);
  void updateKeyboard();
  bool isKeyPressed(const Input &key);
  void updateKeys();
  static InputConstants toKey(const std::string &keyText);
  void drawPause(ngf::RenderTarget &target) const;
  void stopThreads();
  void drawWalkboxes(ngf::RenderTarget &target) const;
  const Verb *getHoveredVerb() const;
  static std::wstring getDisplayName(const std::wstring &name);
  void run(bool state);
  void stopTalking() const;
  void stopTalkingExcept(Entity *pEntity) const;
  Entity *getEntity(Entity *pEntity) const;
  const Verb *overrideVerb(const Verb *pVerb) const;
  void captureScreen(const std::string &path) const;
  void skipText() const;
  void skipCutscene();
  void pauseGame();
  void selectActor(int index);
  void selectPreviousActor();
  void selectNextActor();
  bool hasFlag(int id, uint32_t flagToTest) const;
};
}