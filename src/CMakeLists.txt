include("../appInfo.cmake")

include_directories(${SFML_INCLUDE_DIR})
include_directories(../include/ ../extlibs/squirrel/include/ ../extlibs/spdlog/include/ ../extlibs/clipper/)
link_directories(../extlibs/squirrel/squirrel/ ../extlibs/squirrel/sqstdlib/ ../extlibs/clipper/)

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
        Engine/TextDatabase.cpp
        Engine/Engine.cpp
        Engine/ActorIcons.cpp
        Engine/Inventory.cpp
        Engine/Cutscene.cpp
        Engine/Light.cpp
        Engine/Thread.cpp
        Engine/Camera.cpp
        Engine/EngineSettings.cpp
        Engine/Preferences.cpp
        Engine/Sentence.cpp
        Engine/Callback.cpp
        Engine/Hud.cpp
        Entities/Actor/Actor.cpp
        Entities/Actor/BlinkState.cpp
        Entities/Actor/Costume.cpp
        Entities/Entity.cpp
        Engine/EntityManager.cpp
        Entities/Objects/Object.cpp
        Entities/Objects/TextObject.cpp
        Graphics/FntFont.cpp
        Graphics/GGFont.cpp
        Graphics/ResourceManager.cpp
        Graphics/SpriteSheet.cpp
        Graphics/GraphDrawable.cpp
        Graphics/_WalkboxDrawable.cpp
        Graphics/_Path.cpp
        Graphics/Text.cpp
        Input/CommandManager.cpp
        Input/InputMappings.cpp
        main.cpp
        Parsers/Lip.cpp
        Parsers/YackTokenReader.cpp
        Parsers/YackParser.cpp
        Parsers/GGPack.cpp
        Parsers/GGPackValue.cpp
        Parsers/GGPackBufferStream
        Parsers/JsonTokenReader.cpp
        Parsers/GGHashReader.cpp
        Parsers/GGHashWriter.cpp
        Parsers/SavegameManager.cpp
        Room/Room.cpp
        Room/RoomLayer.cpp
        Room/RoomScaling.cpp
        Room/_RoomTrigger.cpp
        Scripting/ScriptEngine.cpp
        System/_Util.cpp
        System/Logger.cpp
        UI/OptionsDialog.cpp
        UI/QuitDialog.cpp
        UI/SaveLoadDialog.cpp
        UI/StartScreenDialog.cpp
        Util/BTEACrypto.cpp
        Util/RandomNumberGenerator.cpp
        Util/Util.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCES})

# squirrel
target_link_libraries(${PROJECT_NAME} squirrel_static sqstdlib_static)
# clipper
target_link_libraries(${PROJECT_NAME} clipper)
# SFML
target_link_libraries(${PROJECT_NAME} sfml-audio)
target_link_libraries(${PROJECT_NAME} ngf)
# std::filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()


# Install exe
install(TARGETS engge RUNTIME DESTINATION .)
