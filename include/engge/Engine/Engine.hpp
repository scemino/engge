#pragma once
#include "ActorIcons.hpp"
#include "Callback.hpp"
#include "engge/System/NonCopyable.hpp"
#include <squirrel.h>
#include "SavegameSlot.hpp"
#include <memory>
#include <ngf/Graphics/RenderWindow.h>
#include <ngf/Application.h>
#include "engge/Input/InputConstants.hpp"

namespace ng {
class Actor;
class Camera;
class Cutscene;
class DialogManager;
class Entity;
class Function;
class Inventory;
class Object;
class Preferences;
class Room;
class ScriptExecute;
class Sentence;
class SoundDefinition;
class SoundManager;
class ResourceManager;
class ThreadBase;
struct Verb;
class VerbExecute;

enum class UseFlag {
  None = 0,
  UseWith = 1,
  UseOn = 2,
  UseIn = 3,
  GiveTo = 4
};

class Engine : public NonCopyable {
public:
  Engine();
  ~Engine();

  void setApplication(ngf::Application *app);
  [[nodiscard]] const ngf::Application *getApplication() const;

  ResourceManager &getResourceManager();

  Room *getRoom();
  SQInteger setRoom(Room *pRoom);
  SQInteger enterRoomFromDoor(Object *pDoor);
  [[nodiscard]] static std::wstring getText(int id) ;
  [[nodiscard]] static std::wstring getText(const std::string &text) ;

  void addActor(std::unique_ptr<Actor> actor);
  void addRoom(std::unique_ptr<Room> room);
  std::vector<std::unique_ptr<Room>> &getRooms();

  void addCallback(std::unique_ptr<Callback> callback);
  void removeCallback(int id);
  void addFunction(std::unique_ptr<Function> function);
  void setSentence(std::unique_ptr<Sentence> sentence);
  void stopSentence();

  void cutscene(std::unique_ptr<Cutscene> function);
  [[nodiscard]] bool inCutscene() const;
  void cutsceneOverride();
  [[nodiscard]] Cutscene *getCutscene() const;

  std::vector<std::unique_ptr<Actor>> &getActors();

  void update(const ngf::TimeSpan &elapsed);
  void draw(ngf::RenderTarget &target, bool screenshot = false) const;
  [[nodiscard]] int getFrameCounter() const;

  void setCurrentActor(Actor *pCurrentActor, bool userSelected);
  Actor *getCurrentActor();

  void setWalkboxesFlags(int flags);
  [[nodiscard]] int getWalkboxesFlags() const;

  [[nodiscard]] const VerbUiColors *getVerbUiColors(const std::string &name) const;
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute);
  void setDefaultVerb();
  [[nodiscard]] const Verb *getActiveVerb() const;
  void pushSentence(int id, Entity *pObj1, Entity *pObj2);

  void setInputActive(bool active);
  [[nodiscard]] bool getInputActive() const;
  void inputSilentOff();
  void setInputHUD(bool on);
  void setInputVerbs(bool on);

  void setInputState(int state);
  [[nodiscard]] int getInputState() const;

  void follow(Actor *pActor);
  void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute);

  void addThread(std::unique_ptr<ThreadBase> thread);

  std::vector<std::unique_ptr<ThreadBase>> &getThreads();

  void startDialog(const std::string &dialog, const std::string &node);
  void execute(const std::string &code);
  SoundDefinition *getSoundDefinition(const std::string &name);
  bool executeCondition(const std::string &code);
  std::string executeDollar(const std::string &code);

  [[nodiscard]] glm::vec2 getMousePositionInRoom() const;

  Preferences &getPreferences();
  SoundManager &getSoundManager();
  DialogManager &getDialogManager();
  Camera &getCamera();

  Entity *getEntity(const std::string &name);
  void addSelectableActor(int index, Actor *pActor);
  void actorSlotSelectable(Actor *pActor, bool selectable);
  void actorSlotSelectable(int index, bool selectable);
  void setActorSlotSelectable(ActorSlotSelectableMode mode);
  [[nodiscard]] ActorSlotSelectableMode getActorSlotSelectable() const;
  bool isActorSelectable(Actor *pActor) const;
  void setUseFlag(UseFlag flag, Entity *object);
  void flashSelectableActor(bool on);

  [[nodiscard]] ngf::TimeSpan getTime() const;

  HSQOBJECT &getDefaultObject();

  void setFadeAlpha(float fade);
  [[nodiscard]] float getFadeAlpha() const;
  void fadeTo(float destination, ngf::TimeSpan time, InterpolationMethod method);

  void keyDown(const Input& key);
  void keyUp(const Input& key);

  void sayLineAt(glm::ivec2 pos, ngf::Color color, ngf::TimeSpan duration, const std::string &text);
  void sayLineAt(glm::ivec2 pos, Entity &entity, const std::string &text);
  void stopTalking() const;
  void stopTalkingExcept(Entity* pEntity) const;

  void showOptions(bool visible);
  void quit();
  void run();

  Inventory &getInventory();
  Hud &getHud();

  void saveGame(int slot);
  void loadGame(int slot);
  static void getSlotSavegames(std::vector<SavegameSlot> &slots);
  void setAutoSave(bool autosave);
  [[nodiscard]] bool getAutoSave() const;
  void allowSaveGames(bool allow);

private:
  struct Impl;
  std::unique_ptr<Impl> _pImpl;
};
} // namespace ng
