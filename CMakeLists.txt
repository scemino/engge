cmake_minimum_required(VERSION 3.0.0)
project(engge VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)
find_package(SFML REQUIRED system window graphics audio)

include_directories(include/)
link_directories(lib)
add_executable(engge src/Game.cpp src/main.cpp src/GGActor.cpp src/GGAnimation.cpp src/GGCostume.cpp src/GGTextDatabase.cpp src/GGEngine.cpp src/GGFont.cpp src/GGLayer.cpp src/GGObject.cpp src/GGRoom.cpp src/ScriptEngine.cpp src/TextureManager.cpp src/Walkbox.cpp src/GGTextObject.cpp)
target_link_libraries(engge squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(engge ${SFML_LIBRARIES})
endif()
