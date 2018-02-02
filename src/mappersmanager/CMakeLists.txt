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

file(GLOB mappers_list mappers/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/mappersmanager/manager.c src/mappersmanager/utils.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(mappermanager utils.c manager.c ${mappers_list} )
