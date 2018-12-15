cmake_minimum_required(VERSION 3.5.0)
project(engge)

set (CMAKE_CXX_STANDARD 17)
find_package(SFML REQUIRED system window graphics audio)

include_directories(include/)
link_directories(lib)

set(SOURCES
    src/Game.cpp src/main.cpp src/NGActor.cpp src/NGAnimation.cpp src/NGCostume.cpp src/NGCostumeAnimation.cpp
    src/NGTextDatabase.cpp src/NGEngine.cpp src/NGFont.cpp src/NGLayer.cpp src/NGObject.cpp src/NGRoom.cpp
    src/ScriptEngine.cpp src/TextureManager.cpp src/Walkbox.cpp src/NGTextObject.cpp src/RoomLayer.cpp src/NGLip.cpp
    src/SoundDefinition.cpp src/SpriteSheet.cpp src/Dialog/YackTokenReader.cpp src/Dialog/YackParser.cpp 
    src/Dialog/Ast.cpp src/Dialog/DialogManager.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find SFML")
endif()

