cmake_minimum_required(VERSION 3.21)

project(rts_pathfinder
    VERSION 0.1.0
    DESCRIPTION "Path-finding for RTS battle units (software candidate assessment)"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For clangd / clang-tidy tooling. Does not enable clang-tidy in the build itself
# (that's CMAKE_CXX_CLANG_TIDY, deferred until there's source to check).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RTS_BUILD_TESTS "Build unit tests" ON)
option(RTS_BUILD_UI "Build the Qt Quick UI (skipped with a message if Qt6 is absent)" ON)

# Warnings are informational, not build-breaking: a reviewer building this on
# a newer/different compiler should never get a broken build over a new warning.
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

if(RTS_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(third_party)
add_subdirectory(core)

if(RTS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(RTS_BUILD_UI)
    find_package(Qt6 6.5 QUIET COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2)
    if(Qt6_FOUND)
        add_subdirectory(ui)
    else()
        message(STATUS "RTS_BUILD_UI is ON but Qt6 was not found -- skipping ui/ "
                        "(core and tests still build; pass -DCMAKE_PREFIX_PATH=<qt-dir>)")
        set(RTS_BUILD_UI OFF)
    endif()
endif()
