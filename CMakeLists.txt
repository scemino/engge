cmake_minimum_required(VERSION 3.5.0)
project(engge)

set (CMAKE_CXX_STANDARD 17)
find_package(SFML REQUIRED system window graphics audio)

include_directories(include/)
link_directories(lib)

set(SOURCES
    src/Game.cpp src/main.cpp src/Actor.cpp src/Animation.cpp src/Costume.cpp src/CostumeAnimation.cpp
    src/TextDatabase.cpp src/Engine.cpp src/Font.cpp src/Layer.cpp src/Object.cpp src/Room.cpp
    src/Scripting/ScriptEngine.cpp src/TextureManager.cpp src/Walkbox.cpp src/TextObject.cpp src/RoomLayer.cpp src/Lip.cpp
    src/SoundDefinition.cpp src/SpriteSheet.cpp src/Dialog/YackTokenReader.cpp src/Dialog/YackParser.cpp 
    src/Dialog/Ast.cpp src/Dialog/DialogManager.cpp src/Dialog/DialogVisitor.cpp src/FntFont.cpp src/Text.cpp
    src/SoundManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find SFML")
endif()

