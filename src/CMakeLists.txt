include("../appInfo.cmake")

include_directories(../include/ ../extlibs/squirrel/include/ ../extlibs/spdlog/include/ ./)
link_directories(../extlibs/squirrel/squirrel/ ../extlibs/squirrel/sqstdlib/)

set(SOURCES
        Audio/SoundTrigger.cpp
        Audio/SoundId.cpp
        Audio/SoundDefinition.cpp
        Audio/SoundManager.cpp
        Dialog/Ast.cpp
        Dialog/DialogManager.cpp
        Dialog/ConditionVisitor.cpp
        Dialog/ExpressionVisitor.cpp
        Dialog/DialogPlayer.cpp
        Dialog/EngineDialogScript.cpp
        EnggeApplication.cpp
        Engine/AchievementManager.cpp
        Engine/ActorIcons.cpp
        Engine/Callback.cpp
        Engine/Camera.cpp
        Engine/Cutscene.cpp
        Engine/Engine.cpp
        Engine/EngineImpl.cpp
        Engine/EngineSettings.cpp
        Engine/EntityManager.cpp
        Engine/Hud.cpp
        Engine/Inventory.cpp
        Engine/Light.cpp
        Engine/Preferences.cpp
        Engine/Sentence.cpp
        Engine/Shaders.cpp
        Engine/TextDatabase.cpp
        Engine/Thread.cpp
        Engine/ThreadBase.cpp
        Engine/TimeFunction.cpp
        Engine/Trigger.cpp
        Entities/Actor.cpp
        Entities/AnimationLoader.cpp
        Entities/BlinkState.cpp
        Entities/Costume.cpp
        Entities/LipAnimation.cpp
        Entities/JiggleFunction.cpp
        Entities/ShakeFunction.cpp
        Entities/TalkingState.cpp
        Entities/Entity.cpp
        Entities/Object.cpp
        Entities/TextObject.cpp
        Entities/WalkingState.cpp
        Graphics/AnimControl.cpp
        Graphics/AnimDrawable.cpp
        Graphics/GGFont.cpp
        Graphics/ResourceManager.cpp
        Graphics/SpriteSheet.cpp
        Graphics/GraphDrawable.cpp
        Graphics/LightingShader.cpp
        Graphics/WalkboxDrawable.cpp
        Graphics/PathDrawable.cpp
        Graphics/Text.cpp
        Input/CommandManager.cpp
        Input/InputMappings.cpp
        main.cpp
        Parsers/Lip.cpp
        Parsers/YackTokenReader.cpp
        Parsers/YackParser.cpp
        Parsers/GGPackBufferStream
        Parsers/SavegameManager.cpp
        Room/Room.cpp
        Room/RoomLayer.cpp
        Room/RoomScaling.cpp
        Room/RoomTrigger.cpp
        Room/RoomTriggerThread.cpp
        Scripting/ActorWalk.cpp
        Scripting/DefaultScriptExecute.cpp
        Scripting/DefaultVerbExecute.cpp
        Scripting/PostWalk.cpp
        Scripting/ReachAnim.cpp
        Scripting/SetDefaultVerb.cpp
        Scripting/ScriptEngine.cpp
        Scripting/VerbExecuteFunction.cpp
        System/DebugTools/ActorTools.cpp
        System/DebugTools/CameraTools.cpp
        System/DebugTools/Console.cpp
        System/DebugTools/ConsoleTools.cpp
        System/DebugTools/DebugControls.cpp
        System/DebugTools/DebugTools.cpp
        System/DebugTools/GeneralTools.cpp
        System/DebugTools/ObjectTools.cpp
        System/DebugTools/PreferencesTools.cpp
        System/DebugTools/RoomTools.cpp
        System/DebugTools/SoundTools.cpp
        System/DebugTools/TextureTools.cpp
        System/DebugTools/ThreadTools.cpp
        System/Logger.cpp
        UI/Button.cpp
        UI/Checkbox.cpp
        UI/Control.cpp
        UI/HelpDialog.cpp
        UI/OptionsDialog.cpp
        UI/QuitDialog.cpp
        UI/SaveLoadDialog.cpp
        UI/StartScreenDialog.cpp
        UI/Slider.cpp
        UI/SwitchButton.cpp
        Util/BTEACrypto.cpp
        Util/RandomNumberGenerator.cpp
        Util/Util.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCES})

# squirrel
target_link_libraries(${PROJECT_NAME} squirrel_static sqstdlib_static)
# clipper
target_link_libraries(${PROJECT_NAME} clipper)
# ngf
target_link_libraries(${PROJECT_NAME} ngf)
# std::filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif ()


# Install exe
install(TARGETS engge RUNTIME DESTINATION "bin")
