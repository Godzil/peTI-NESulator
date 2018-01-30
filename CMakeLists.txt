#
# TI-NES CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2008 986Corp. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

cmake_minimum_required (VERSION 2.8)

project (TINES)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("external/glfw")
find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR})

if (COVERALLS)
 enable_testing()
endif (COVERALLS)

add_subdirectory (src)
