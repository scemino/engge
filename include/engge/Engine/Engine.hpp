#pragma once
#include <memory>
#include <squirrel.h>
#include <ngf/Graphics/RenderWindow.h>
#include <ngf/Application.h>
#include <engge/Engine/ActorIcons.hpp>
#include <engge/Engine/Callback.hpp>
#include <engge/Engine/RoomEffect.hpp>
#include <engge/Engine/SavegameSlot.hpp>
#include <engge/System/NonCopyable.hpp>
#include <engge/Input/InputConstants.hpp>

namespace ng {
class Actor;
class Camera;
class Cutscene;
class DialogManager;
class EnggeApplication;
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

enum class FadeEffect {
  None,
  In,
  Out,
  Wobble
};

struct FadeEffectParameters {
  FadeEffect effect{FadeEffect::None};
  Room *room{nullptr};
  ngf::TimeSpan duration;
  ngf::TimeSpan elapsed;
  glm::vec2 cameraTopLeft{0, 0};
  float movement{0.f};
  bool fadeToSepia{false};
};

enum class WalkboxesFlags {
  None = 0,
  Merged = 1,
  Graph = 2,
  Walkboxes = 4,
};

class Engine : public NonCopyable {
public:
  Engine();
  ~Engine();

  void setApplication(ng::EnggeApplication *app);
  [[nodiscard]] const ng::EnggeApplication *getApplication() const;
  [[nodiscard]] ng::EnggeApplication *getApplication();

  Room *getRoom();
  SQInteger setRoom(Room *pRoom);
  SQInteger enterRoomFromDoor(Object *pDoor);

  [[nodiscard]] static std::wstring getText(int id);
  [[nodiscard]] static std::wstring getText(const std::string &text);

  void addRoom(std::unique_ptr<Room> room);
  std::vector<std::unique_ptr<Room>> &getRooms();

  void addActor(std::unique_ptr<Actor> actor);
  std::vector<std::unique_ptr<Actor>> &getActors();
  void setCurrentActor(Actor *pCurrentActor, bool userSelected);
  Actor *getCurrentActor();
  Entity *getEntity(const std::string &name);

  void addCallback(std::unique_ptr<Callback> callback);
  void removeCallback(int id);

  void addFunction(std::unique_ptr<Function> function);
  void cutscene(std::unique_ptr<Cutscene> function);
  [[nodiscard]] bool inCutscene() const;
  void cutsceneOverride();
  [[nodiscard]] Cutscene *getCutscene() const;

  void update(const ngf::TimeSpan &elapsed);
  void draw(ngf::RenderTarget &target, bool screenshot = false) const;
  [[nodiscard]] int getFrameCounter() const;

  void setWalkboxesFlags(WalkboxesFlags flags);
  [[nodiscard]] WalkboxesFlags getWalkboxesFlags() const;

  [[nodiscard]] const VerbUiColors *getVerbUiColors(const std::string &name) const;
  void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute);
  void setDefaultVerb();
  [[nodiscard]] const Verb *getActiveVerb() const;
  void pushSentence(int id, Entity *pObj1, Entity *pObj2);
  void setSentence(std::unique_ptr<Sentence> sentence);
  void stopSentence();

  void setInputActive(bool active);
  [[nodiscard]] bool getInputActive() const;
  void inputSilentOff();
  void setInputHUD(bool on);
  void setInputVerbs(bool on);

  void setInputState(int state);
  [[nodiscard]] int getInputState() const;

  void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute);

  void addThread(std::unique_ptr<ThreadBase> thread);
  std::vector<std::unique_ptr<ThreadBase>> &getThreads();

  void startDialog(const std::string &dialog, const std::string &node);
  void execute(const std::string &code);
  bool executeCondition(const std::string &code);
  std::string executeDollar(const std::string &code);

  SoundDefinition *getSoundDefinition(const std::string &name);

  [[nodiscard]] glm::vec2 getMousePositionInRoom() const;

  Preferences &getPreferences();
  SoundManager &getSoundManager();
  DialogManager &getDialogManager();
  ResourceManager &getResourceManager();

  Camera &getCamera();
  void follow(Actor *pActor);
  const Actor *getFollowActor() const;

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

  /// Fades the screen with the specified effect and duration.
  /// \param effect: Effect to use to fade the screen.
  /// \param duration: Duration of the effect.
  void fadeTo(FadeEffect effect, const ngf::TimeSpan &duration);
  FadeEffectParameters &getFadeParameters();

  void keyDown(const Input &key);
  void keyUp(const Input &key);

  void sayLineAt(glm::ivec2 pos, ngf::Color color, ngf::TimeSpan duration, const std::string &text);
  void sayLineAt(glm::ivec2 pos, Entity &entity, const std::string &text);
  void stopTalking() const;
  void stopTalkingExcept(Entity *pEntity) const;

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

public:
  RoomEffect roomEffect;

private:
  struct Impl;
  std::unique_ptr<Impl> m_pImpl;
};
} // namespace ng
