cmake_minimum_required(VERSION 3.5)

include(appInfo.cmake)

project("${appName}" LANGUAGES CXX)

# Compiler flags
set (SQ_DISABLE_INSTALLER ON)

# Dependencies
find_package (SFML REQUIRED COMPONENTS system window graphics audio)
find_package (ImGui-SFML REQUIRED)

# Sources
add_subdirectory(src)
add_subdirectory(extlibs/squirrel)

# Install misc. files
install(FILES LICENSE DESTINATION .)

# Install libraries on Linux
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

    install(FILES "${ImGui-SFML_DIR}/../../libImGui-SFML.so" DESTINATION .)
    install(DIRECTORY "${SFML_DIR}/../../" DESTINATION . FILES_MATCHING PATTERN "libsfml*")

    install(CODE "execute_process(COMMAND patchelf --remove-needed libImGui-SFML.so ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --add-needed ./libImGui-SFML.so ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --remove-needed libsfml-audio.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --add-needed ./libsfml-audio.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --remove-needed libsfml-system.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --add-needed ./libsfml-system.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --remove-needed libsfml-window.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --add-needed ./libsfml-window.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --remove-needed libsfml-graphics.so.2.5 ./src/engge)")
    install(CODE "execute_process(COMMAND patchelf --add-needed ./libsfml-graphics.so.2.5 ./src/engge)")

# Install libraries on Windows
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")

    install(FILES "${ImGui-SFML_DIR}/../../../bin/ImGui-SFML.dll" DESTINATION .)
    install(FILES "${SFML_DIR}/../../../bin/openal32.dll" DESTINATION .)
    install(FILES "${SFML_DIR}/../../../bin/sfml-audio-2.dll" DESTINATION .)
    install(FILES "${SFML_DIR}/../../../bin/sfml-graphics-2.dll" DESTINATION .)
    install(FILES "${SFML_DIR}/../../../bin/sfml-system-2.dll" DESTINATION .)
    install(FILES "${SFML_DIR}/../../../bin/sfml-window-2.dll" DESTINATION .)

endif()

target_compile_features("${appName}" PRIVATE cxx_std_17)
set_target_properties("${appName}" PROPERTIES CXX_EXTENSIONS OFF)

if (MSVC)
    # TODO: warning level 4 and all warnings as errors
    # target_compile_options("${appName}" PRIVATE /W4 /WX)
else()
    # lots of warnings and all warnings as errors
    # TODO: treat warnings as errors: -Werror
    target_compile_options("${appName}" PRIVATE -Wall -Wextra -pedantic-errors)
endif()

# Configure CPack
function(get_short_system_name variable)
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
		set(${variable} "OSX" PARENT_SCOPE)
	elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
		set(${variable} "Win32" PARENT_SCOPE)
	else()
		set(${variable} "${CMAKE_SYSTEM_NAME}" PARENT_SCOPE)
	endif()
endfunction()

set(CPACK_PACKAGE_NAME ${appName})
string(REPLACE " " "-" CPACK_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
get_short_system_name(CPACK_SYSTEM_NAME)
set(CPACK_PACKAGE_VERSION_MAJOR ${appVersionMajor})
set(CPACK_PACKAGE_VERSION_MINOR ${appVersionMinor})
set(CPACK_PACKAGE_VERSION_PATCH ${appVersionPatch})
set(CPACK_PACKAGE_VERSION ${appVersion})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
string(TOLOWER "${CPACK_PACKAGE_FILE_NAME}" CPACK_PACKAGE_FILE_NAME)
set(CPACK_GENERATOR ZIP)

# Run CPack
include(CPack)

