cmake_minimum_required(VERSION 3.5.0)
project(engge)

set (CMAKE_CXX_STANDARD 17)
find_package(SFML REQUIRED system window graphics audio)

include_directories(include/)
link_directories(lib)

set(SOURCES
    src/Game.cpp src/main.cpp src/GGActor.cpp src/GGAnimation.cpp src/GGCostume.cpp src/GGCostumeAnimation.cpp
    src/GGTextDatabase.cpp src/GGEngine.cpp src/GGFont.cpp src/GGLayer.cpp src/GGObject.cpp src/GGRoom.cpp
    src/ScriptEngine.cpp src/TextureManager.cpp src/Walkbox.cpp src/GGTextObject.cpp src/RoomLayer.cpp src/GGLip.cpp
    src/SoundDefinition.cpp src/SpriteSheet.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${SFML_LIBRARIES})
else()
    message (FATAL_ERROR "Cannot find SFML")
endif()

