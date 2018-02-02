#
# peTI-NESulator CMake
#
# Created by Manoël TRAPIER.
# Copyright (c) 2003-2018 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

file(GLOB ppu_debug_list debug/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/ppu/ppu.c src/ppu/ppu.memory.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()


add_library(ppu ppu.c ppu.memory.c ${ppu_debug_list})
