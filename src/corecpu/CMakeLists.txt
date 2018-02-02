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

if (COVERALLS)
    set(COVERAGE_SRCS src/corecpu/corecpu.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(corecpu corecpu.c)
