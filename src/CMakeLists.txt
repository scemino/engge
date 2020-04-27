include("../appInfo.cmake")

include_directories(../include/ ../extlibs/squirrel/include/ ../extlibs/spdlog/include/ ../extlibs/clipper/)
link_directories(../extlibs/squirrel/squirrel/ ../extlibs/squirrel/sqstdlib/ ../extlibs/clipper/)

set(SOURCES
        Game.cpp main.cpp Entities/Actor/Actor.cpp Entities/Objects/Animation.cpp Entities/Actor/Costume.cpp Entities/Actor/CostumeAnimation.cpp
        Engine/TextDatabase.cpp Engine/Engine.cpp Font/GGFont.cpp Entities/Actor/CostumeLayer.cpp Entities/Objects/Object.cpp Room/Room.cpp
        Scripting/ScriptEngine.cpp Graphics/TextureManager.cpp Math/PathFinding/Walkbox.cpp Entities/Objects/TextObject.cpp Room/RoomLayer.cpp Parsers/Lip.cpp
        Audio/SoundDefinition.cpp Graphics/SpriteSheet.cpp Parsers/YackTokenReader.cpp Parsers/YackParser.cpp
        Dialog/Ast.cpp Dialog/DialogManager.cpp Dialog/DialogVisitor.cpp Font/FntFont.cpp Graphics/Text.cpp
        Audio/SoundManager.cpp Engine/ActorIcons.cpp Engine/Inventory.cpp Math/PathFinding/Graph.cpp Math/PathFinding/PathFinder.cpp Parsers/GGPack.cpp  Parsers/GGPackValue.cpp Parsers/GGPackBufferStream
        Engine/Cutscene.cpp Entities/Entity.cpp Room/RoomScaling.cpp Audio/SoundTrigger.cpp Audio/SoundId.cpp Parsers/JsonTokenReader.cpp
        Engine/Light.cpp Engine/Thread.cpp System/_Util.cpp Engine/Camera.cpp System/Logger.cpp Engine/EngineSettings.cpp Engine/Preferences.cpp
        Engine/Sentence.cpp Room/_RoomTrigger.cpp Entities/Actor/BlinkState.cpp Engine/Callback.cpp Math/PathFinding/_IndexedPriorityQueue.cpp
        Math/PathFinding/GraphEdge.cpp Math/PathFinding/_Path.cpp Math/PathFinding/_AstarAlgorithm.cpp Math/PathFinding/_WalkboxDrawable.cpp
        Math/Segment.cpp UI/OptionsDialog.cpp UI/QuitDialog.cpp UI/SaveLoadDialog.cpp Entities/Objects/AnimationFrame.cpp UI/StartScreenDialog.cpp Engine/Hud.cpp ../include/Engine/Hud.hpp ../include/Parsers/GGHashReader.hpp Parsers/GGHashReader.cpp ../include/Parsers/GGHashWriter.hpp Parsers/GGHashWriter.cpp ../include/Parsers/SavegameManager.hpp Parsers/SavegameManager.cpp ../include/Engine/ExCommandConstants.hpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# squirrel
target_link_libraries(${PROJECT_NAME} squirrel_static sqstdlib_static)
# clipper
target_link_libraries(${PROJECT_NAME} clipper)
# SFML
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else ()
    message(FATAL_ERROR "Cannot find SFML")
endif ()
# ImGui-SFML
target_link_libraries(${PROJECT_NAME} ImGui-SFML::ImGui-SFML)

# Install exe
install(TARGETS engge RUNTIME DESTINATION .)
