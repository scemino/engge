#pragma once
#include "ActorIcons.h"
#include "Callback.h"
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

namespace InputConstants
{
// these codes corresponds to SDL key codes used in TWP
static const int KEY_UP    = 0x40000052;
static const int KEY_RIGHT = 0x4000004F;
static const int KEY_DOWN  = 0x40000051;
static const int KEY_LEFT  = 0x40000050;
static const int KEY_PAD1  = 0x40000059;
static const int KEY_PAD2  = 0x4000005A;
static const int KEY_PAD3  = 0x4000005B;
static const int KEY_PAD4  = 0x4000005C;
static const int KEY_PAD5  = 0x4000005D;
static const int KEY_PAD6  = 0x4000005E;
static const int KEY_PAD7  = 0x4000005F;
static const int KEY_PAD8  = 0x40000056;
static const int KEY_PAD9  = 0x40000061;
static const int KEY_ESCAPE     = 0x08;
static const int KEY_TAB        = 0x09;
static const int KEY_RETURN     = 0x0D;
static const int KEY_BACKSPACE  = 0x1B;
static const int KEY_SPACE = 0X20;
static const int KEY_A  = 0x61;
static const int KEY_B  = 0x62;
static const int KEY_C  = 0x63;
static const int KEY_D  = 0x64;
static const int KEY_E  = 0x65;
static const int KEY_F  = 0x66;
static const int KEY_G  = 0x67;
static const int KEY_H  = 0x68;
static const int KEY_I  = 0x69;
static const int KEY_J  = 0x6A;
static const int KEY_K  = 0x6B;
static const int KEY_L  = 0x6C;
static const int KEY_M  = 0x6D;
static const int KEY_N  = 0x6E;
static const int KEY_O  = 0x6F;
static const int KEY_P  = 0x70;
static const int KEY_Q  = 0x71;
static const int KEY_R  = 0x72;
static const int KEY_S  = 0x73;
static const int KEY_T  = 0x74;
static const int KEY_U  = 0x75;
static const int KEY_V  = 0x76;
static const int KEY_W  = 0x77;
static const int KEY_X  = 0x78;
static const int KEY_Y  = 0x79;
static const int KEY_Z  = 0x7A;
static const int KEY_0  = 0x30;
static const int KEY_1  = 0x31;
static const int KEY_2  = 0x32;
static const int KEY_3  = 0x33;
static const int KEY_4  = 0x34;
static const int KEY_5  = 0x35;
static const int KEY_6  = 0x36;
static const int KEY_7  = 0x37;
static const int KEY_8  = 0x38;
static const int KEY_9  = 0x39;
static const int KEY_F1  = 0x4000003A;
static const int KEY_F2  = 0x4000003B;
static const int KEY_F3  = 0x4000003C;
static const int KEY_F4  = 0x4000003D;
static const int KEY_F5  = 0x4000003E;
static const int KEY_F6  = 0x4000003F;
static const int KEY_F7  = 0x40000040;
static const int KEY_F8  = 0x40000041;
static const int KEY_F9  = 0x40000042;
static const int KEY_F10 = 0x40000043;
static const int KEY_F11 = 0x40000044;
static const int KEY_F12 = 0x40000045;

static const int BUTTON_A     = 0x3E8;
static const int BUTTON_B     = 0x3E9;
static const int BUTTON_X     = 0x3EA;
static const int BUTTON_Y     = 0x3EB;
static const int BUTTON_START = 0x3EC;
static const int BUTTON_BACK  = 0x3EC;
} // namespace InputConstants

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

enum class CursorDirection : unsigned int
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
    [[nodiscard]] const sf::RenderWindow &getWindow() const;

    TextureManager &getTextureManager();
    EngineSettings &getSettings();

    Room *getRoom();
    SQInteger setRoom(Room *pRoom);
    SQInteger enterRoomFromDoor(Object *pDoor);
    [[nodiscard]] std::wstring getText(int id) const;
    [[nodiscard]] std::wstring getText(const std::string& text) const;

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
    [[nodiscard]] Cutscene* getCutscene() const ;

    std::vector<std::unique_ptr<Actor>> &getActors();

    void update(const sf::Time &elapsed);
    void draw(sf::RenderWindow &window) const;
    [[nodiscard]] int getFrameCounter() const;

    void setCurrentActor(Actor *pCurrentActor, bool userSelected);
    Actor *getCurrentActor();
    [[nodiscard]] bool actorShouldRun() const;

    void setWalkboxesFlags(int flags);
    [[nodiscard]] int getWalkboxesFlags() const;

    void setVerb(int characterSlot, int verbSlot, const Verb &verb);
    void setVerbUiColors(int characterSlot, VerbUiColors colors);
    const VerbUiColors *getVerbUiColors(const std::string& name) const;
    void setVerbExecute(std::unique_ptr<VerbExecute> verbExecute);
    [[nodiscard]] const Verb *getVerb(int id) const;
    void setDefaultVerb();
    [[nodiscard]] const Verb *getActiveVerb() const;
    void pushSentence(int id, Entity* pObj1, Entity* pObj2);

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
    void stopThread(int threadId);
    std::vector<std::unique_ptr<ThreadBase>>& getThreads();

    void startDialog(const std::string &dialog, const std::string &node);
    void execute(const std::string &code);
    SoundDefinition *getSoundDefinition(const std::string &name);
    bool executeCondition(const std::string &code);
    std::string executeDollar(const std::string &code);

    [[nodiscard]] sf::Vector2f getMousePos() const;
    [[nodiscard]] sf::Vector2f getMousePositionInRoom() const;
    [[nodiscard]] sf::Vector2f findScreenPosition(int verb) const;

    Preferences &getPreferences();
    SoundManager &getSoundManager();
    DialogManager &getDialogManager();
    Camera &getCamera();

    void addSelectableActor(int index, Actor *pActor);
    void actorSlotSelectable(Actor *pActor, bool selectable);
    void actorSlotSelectable(int index, bool selectable);
    void setActorSlotSelectable(ActorSlotSelectableMode mode);
    bool isActorSelectable(Actor* pActor) const;
    void setUseFlag(UseFlag flag, Entity *object);
    void flashSelectableActor(bool on);

    [[nodiscard]] sf::Time getTime() const;

    void setVm(HSQUIRRELVM vm);
    HSQUIRRELVM getVm();

    HSQOBJECT &getDefaultObject();

    void setFadeAlpha(float fade);
    [[nodiscard]] float getFadeAlpha() const;
    void fadeTo(float destination, sf::Time time, InterpolationMethod method);

    void keyDown(int key);
    void keyUp(int key);

    void setRanges(sf::Vector2f ranges);
    [[nodiscard]] sf::Vector2f getRanges() const;
    void setVerbColor(sf::Color color);
    [[nodiscard]] sf::Color getVerbColor() const;
    void setVerbShadowColor(sf::Color color);
    [[nodiscard]] sf::Color getVerbShadowColor() const;
    void setVerbNormalColor(sf::Color color);
    [[nodiscard]] sf::Color getVerbNormalColor() const;
    void setVerbHighlightColor(sf::Color color);
    [[nodiscard]] sf::Color getVerbHighlightColor() const;

    void sayLineAt(sf::Vector2i pos, sf::Color color, sf::Time duration, const std::string& text);
    void sayLineAt(sf::Vector2i pos, Actor& actor, const std::string& text);

  private:
    struct Impl;
    std::unique_ptr<Impl> _pImpl;
};
} // namespace ng
