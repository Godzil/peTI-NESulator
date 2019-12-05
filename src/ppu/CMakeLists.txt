#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
file(GLOB ppu_debug_list debug/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/ppu/ppu.c src/ppu/ppu.memory.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()


add_library(ppu ppu.c ppu.memory.c ${ppu_debug_list})
