#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
if (COVERALLS)
    set(COVERAGE_SRCS src/memorymanager/memory.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(memorymanager memory.c)
