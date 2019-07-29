cmake_minimum_required(VERSION 3.5.0)
project(engge)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
find_package( SFML REQUIRED COMPONENTS system window graphics audio )

include_directories(include/ extlibs/squirrel/include/ extlibs/spdlog/include/)
link_directories(extlibs/squirrel/squirrel/ extlibs/squirrel/sqstdlib/)

set(SOURCES
    src/Game.cpp src/main.cpp src/Actor.cpp src/Animation.cpp src/Costume.cpp src/CostumeAnimation.cpp
    src/TextDatabase.cpp src/Engine.cpp src/Font.cpp src/CostumeLayer.cpp src/Object.cpp src/Room.cpp
    src/Scripting/ScriptEngine.cpp src/TextureManager.cpp src/Walkbox.cpp src/TextObject.cpp src/RoomLayer.cpp src/Lip.cpp
    src/SoundDefinition.cpp src/SpriteSheet.cpp src/Dialog/YackTokenReader.cpp src/Dialog/YackParser.cpp 
    src/Dialog/Ast.cpp src/Dialog/DialogManager.cpp src/Dialog/DialogVisitor.cpp src/FntFont.cpp src/Text.cpp
    src/SoundManager.cpp src/ActorIcons.cpp src/Inventory.cpp src/Graph.cpp src/PathFinder.cpp src/GGPack.cpp
    src/Cutscene.cpp src/Entity.cpp src/RoomScaling.cpp src/SoundTrigger.cpp src/SoundId.cpp src/JsonTokenReader.cpp
    src/Light.cpp src/Thread.cpp src/_Util.cpp src/Camera.cpp src/Logger.cpp
)

add_subdirectory(extlibs/squirrel)
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find SFML")
endif()

