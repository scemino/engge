#pragma once
#include "ActorIcons.h"
#include "NonCopyable.h"
#include "SFML/Graphics.hpp"
#include "squirrel.h"
#include <memory>

namespace ng
{
namespace InputStateConstants
{
static const int UI_INPUT_ON = 1;
static const int UI_INPUT_OFF = 2;
static const int UI_VERBS_ON = 4;
static const int UI_VERBS_OFF = 8;
static const int UI_CURSOR_ON = 0x40;
static const int UI_CURSOR_OFF = 0x80;
static const int UI_HUDOBJECTS_ON = 0x10;
static const int UI_HUDOBJECTS_OFF = 0x20;
}

class Actor;
class Camera;
class Cutscene;
class DialogManager;
class EngineSettings;
class Entity;
class Function;
class Object;
class Preferences;
class Room;
class ScriptExecute;
class Sentence;
class SoundDefinition;
class SoundManager;
class TextureManager;
class ThreadBase;
struct Verb;
class VerbExecute;

enum class CursorDirection
{
    None = 0,
    Left = 1,
    Right = 1u << 1u,
    Up = 1u << 2u,
    Down = 1u << 3u,
    Hotspot = 1u << 4u
};

enum class UseFlag
{
    None = 0,
    UseWith = 1,
    UseOn = 2,
    UseIn = 3,
    GiveTo = 4
};

class Engine : public NonCopyable
{
  public:
    explicit Engine(EngineSettings &settings);
    ~Engine();

    void setWindow(sf::RenderWindow &window);
    const sf::RenderWindow &getWindow() const;

    TextureManager &getTextureManager();
    EngineSettings &getSettings();

    Room *getRoom();
    SQInteger setRoom(Room *pRoom);
    SQInteger enterRoomFromDoor(Object *pDoor);
    std::wstring getText(int id) const;

    void addActor(std::unique_ptr<Actor> actor);
    void addRoom(std::unique_ptr<Room> room);
    const std::vector<std::unique_ptr<Room>> &getRooms() const;
    std::vector<std::unique_ptr<Room>> &getRooms();
    
    void addFunction(std::unique_ptr<Function> function);
    void setSentence(std::unique_ptr<Sentence> sentence);
    void stopSentence();

    void cutscene(std::unique_ptr<Cutscene> function);
    bool inCutscene() const;
    void cutsceneOverride();

    std::vector<std::unique_ptr<Actor>> &getActors();

    void update(const sf::Time &elapsed);
    void draw(sf::RenderWindow &window) const;
    int getFrameCounter() const;

    void setCurrentActor(Actor *pCurrentActor, bool userSelected);
    Actor *getCurrentActor();
    Actor *getFollowActor();

    void setVerb(int characterSlot, int verbSlot, const Verb &verb);
    void setVerbUiColors(int characterSlot, VerbUiColors colors);
    VerbUiColors &getVerbUiColors(int characterSlot);
    void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute);
    const Verb *getVerb(int id) const;
    void setDefaultVerb();
    const Verb *getActiveVerb() const;
    void pushSentence(int id, Entity* pObj1, Entity* pObj2);

    void setInputActive(bool active);
    bool getInputActive() const;
    void inputSilentOff();
    bool isCursorVisible() const;

    void setInputHUD(bool on);
    bool getInputHUD() const;
    
    void setInputVerbs(bool on);
    bool getInputVerbs() const;
    
    void setInputState(int state);
    int getInputState() const;

    void follow(Actor *pActor);
    void setScriptExecute(std::unique_ptr<ScriptExecute> scriptExecute);
    
    void addThread(std::unique_ptr<ThreadBase> thread);
    void stopThread(HSQUIRRELVM thread);
    bool isThreadAlive(HSQUIRRELVM thread) const;

    void startDialog(const std::string &dialog, const std::string &node);
    void execute(const std::string &code);
    SoundDefinition *getSoundDefinition(const std::string &name);
    bool executeCondition(const std::string &code);
    std::string executeDollar(const std::string &code);

    sf::Vector2f getMousePos() const;

    Preferences &getPreferences();
    SoundManager &getSoundManager();
    DialogManager &getDialogManager();
    Camera &getCamera();

    void addSelectableActor(int index, Actor *pActor);
    void actorSlotSelectable(Actor *pActor, bool selectable);
    void actorSlotSelectable(int index, bool selectable);
    void setActorSlotSelectable(ActorSlotSelectableMode mode);
    void setUseFlag(UseFlag flag, Entity *object);
    void flashSelectableActor(bool on);

    sf::Time getTime() const;

    void setVm(HSQUIRRELVM vm);
    HSQUIRRELVM getVm();

    HSQOBJECT &getDefaultObject();

    void setShader(const std::string& code);
    std::string getShader() const;

    void setFadeAlpha(float fade);
    float getFadeAlpha() const;
    void fadeTo(float destination, sf::Time time, InterpolationMethod method);

  private:
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
};
} // namespace ng
