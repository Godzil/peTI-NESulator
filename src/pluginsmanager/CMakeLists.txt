#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#


file(GLOB plugins_list plugins/*.c)

if (COVERALLS)
    set(COVERAGE_SRCS src/pluginsmanager/manager.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(pluginsmanager manager.c ${plugins_list})
