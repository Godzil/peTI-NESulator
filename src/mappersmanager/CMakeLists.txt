#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#


file(GLOB mappers_list mappers/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/mappersmanager/manager.c src/mappersmanager/utils.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(mappermanager utils.c manager.c ${mappers_list} )
