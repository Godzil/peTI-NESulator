#
# peTI-NESulator CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2002-2019 986-Studio.
#
cmake_minimum_required (VERSION 3.1)

project (peTI-NESulator)

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})

if (USE_GLFW)
    message("Coucou ${USE_GLFW}")
    # Include GLFW
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("external/glfw")
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
endif (USE_GLFW)

if (USE_PORTAUDIO)
    # Include PortAudio
    set(PA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(PA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(PA_ENABLE_DEBUG_OUTPUT OFF CACHE BOOL "" FORCE)
    add_subdirectory("external/portaudio")
endif (USE_PORTAUDIO)

if (COVERALLS)
    enable_testing()
endif (COVERALLS)

# Generate version from Git Tag
include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_MICRO "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" VERSION_SHA1 "${VERSION}")
message("-- Building version ${VERSION}")
add_compile_options(-DV_MAJOR=${VERSION_MAJOR})
add_compile_options(-DV_MINOR=${VERSION_MINOR})
add_compile_options(-DV_MICRO=${VERSION_MICRO})
add_compile_options(-DV_TEXT="${VERSION_SHA1}")

add_subdirectory (src)
