set(test_sources
    tests.cpp
)

add_executable(tests ${test_sources})
target_link_libraries(tests engge_core squirrel sqstdlib sfml-graphics sfml-window sfml-system sfml-audio)
target_include_directories(tests PRIVATE
    ${CMAKE_SOURCE_DIR}/extlibs/catch
)

add_test(all-tests tests)