cmake_minimum_required(VERSION 3.12)
project(canfix_tests CXX)

set(CMAKE_CXX_STANDARD 14)

# ── GoogleTest via FetchContent ───────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.12.1
    GIT_SHALLOW    TRUE
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# ── canfix library (compiled with mock headers) ───────────────────────────────
add_library(canfix_lib STATIC
    ../canfix.cpp
    mock_impl.cpp
)
target_include_directories(canfix_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks   # provides can.h, EEPROM.h
    ${CMAKE_CURRENT_SOURCE_DIR}/..      # provides canfix.h
)

# ── Test executable ───────────────────────────────────────────────────────────
add_executable(canfix_tests
    test_canfix.cpp
)
target_include_directories(canfix_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(canfix_tests
    canfix_lib
    GTest::gtest_main
)

# ── CTest integration ─────────────────────────────────────────────────────────
include(GoogleTest)
gtest_discover_tests(canfix_tests)
