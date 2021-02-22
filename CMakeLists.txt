cmake_minimum_required(VERSION 3.5)

include(appInfo.cmake)

project("${appName}" LANGUAGES CXX)

# Compiler flags
set (SQ_DISABLE_INSTALLER ON)
set (NGF_BUILD_DEMOS OFF)
set (NGF_BUILD_EXAMPLES OFF)
set (NGF_BUILD_TESTS OFF)
set (NGF_BUILD_DOCUMENTATION OFF)

# Sources
add_subdirectory(src)
add_subdirectory(extlibs/squirrel)
add_subdirectory(extlibs/ngf/)

# Install misc. files
install(FILES LICENSE DESTINATION .)

# Copy dependencies
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	install(FILES "${VCPKG_BIN_DIR}/glew32.dll" DESTINATION "bin/")
	install(FILES "${VCPKG_BIN_DIR}/SDL2.dll" DESTINATION "bin/")
endif()

target_compile_features("${appName}" PRIVATE cxx_std_17)
set_target_properties("${appName}" PROPERTIES CXX_EXTENSIONS OFF)

if (MSVC)
    # TODO: warning level 4 and all warnings as errors
    # target_compile_options("${appName}" PRIVATE /W4 /WX)
else()
    # lots of warnings and all warnings as errors
    # TODO: treat warnings as errors: -Werror
    # -pedantic-errors reports error library {fmt}
    target_compile_options("${appName}" PRIVATE -Wall -Wextra)
endif()

# Configure CPack
function(get_short_system_name variable)
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
		set(${variable} "OSX" PARENT_SCOPE)
	elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
		set(${variable} "Win64" PARENT_SCOPE)
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

