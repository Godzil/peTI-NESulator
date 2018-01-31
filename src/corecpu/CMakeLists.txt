#
# peTI-NESulator CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2008 986Corp. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

if (COVERALLS)
    set(COVERAGE_SRCS src/corecpu/corecpu.c ${COVERAGE_SRCS} PARENT_SCOPE)
endif()

add_library(corecpu corecpu.c)
