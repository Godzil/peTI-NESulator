#
# TI-NES CMake
#
# Created by Manoel TRAPIER.
# Copyright (c) 2003-2016 986-Studio. All rights reserved.
#
# $LastChangedDate$
# $Author$
# $HeadURL$
# $Revision$

if (COVERALLS)
    set(COVERAGE_SRCS src/os/unix/loadfile.c src/os/unix/graphics_dummy.c src/os/unix/sound.c src/os/unix/io.c ${COVERAGE_SRCS} PARENT_SCOPE)
    add_library(oslib loadfile.c graphics_dummy.c sound.c io.c)
else()
    add_library(oslib loadfile.c graphics.c sound.c io.c)
endif()

target_link_libraries(oslib glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
