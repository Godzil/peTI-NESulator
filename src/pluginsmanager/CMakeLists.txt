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

file(GLOB plugins_list plugins/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/pluginsmanager/manager.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(pluginsmanager manager.c ${plugins_list})
