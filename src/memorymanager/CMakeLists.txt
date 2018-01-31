#
# peTI-NESulator CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2018 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

if (COVERALLS)
    set(COVERAGE_SRCS src/memorymanager/memory.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(memorymanager memory.c)
