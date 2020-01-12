include("../appInfo.cmake")

include_directories(../include/ ../extlibs/squirrel/include/ ../extlibs/spdlog/include/)
link_directories(../extlibs/squirrel/squirrel/ ../extlibs/squirrel/sqstdlib/)

set(SOURCES
    Game.cpp main.cpp Actor.cpp Animation.cpp Costume.cpp CostumeAnimation.cpp
    TextDatabase.cpp Engine.cpp Font.cpp CostumeLayer.cpp Object.cpp Room.cpp
    Scripting/ScriptEngine.cpp TextureManager.cpp PathFinding/Walkbox.cpp TextObject.cpp RoomLayer.cpp Lip.cpp
    Audio/SoundDefinition.cpp SpriteSheet.cpp Dialog/YackTokenReader.cpp Dialog/YackParser.cpp 
    Dialog/Ast.cpp Dialog/DialogManager.cpp Dialog/DialogVisitor.cpp FntFont.cpp Text.cpp
    Audio/SoundManager.cpp ActorIcons.cpp Inventory.cpp PathFinding/Graph.cpp PathFinding/PathFinder.cpp GGPack.cpp
    Cutscene.cpp Entity.cpp RoomScaling.cpp Audio/SoundTrigger.cpp Audio/SoundId.cpp JsonTokenReader.cpp
    Light.cpp Thread.cpp _Util.cpp Camera.cpp Logger.cpp EngineSettings.cpp Preferences.cpp
    Sentence.cpp _RoomTrigger.cpp BlinkState.cpp Callback.cpp PathFinding/_IndexedPriorityQueue.cpp
    PathFinding/GraphEdge.cpp PathFinding/_Path.cpp PathFinding/_AstarAlgorithm.cpp PathFinding/_WalkboxDrawable.cpp
        clipper.cpp Segment.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} squirrel_static sqstdlib_static sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find SFML")
endif()
target_link_libraries(${PROJECT_NAME} ImGui-SFML::ImGui-SFML)

# Install exe
install(TARGETS engge RUNTIME DESTINATION .)
