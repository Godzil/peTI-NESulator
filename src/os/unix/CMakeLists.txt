#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
set(USE_GLFW ON BOOL CACHE INTERNAL "USE_GLFW")
set(USE_PORTAUDIO ON BOOL CACHE INTERNAL "USE_PORTAUDIO")

if (COVERALLS)
    set(COVERAGE_SRCS src/os/unix/loadfile.c src/os/unix/graphics_dummy.c src/os/unix/sound.c src/os/unix/io.c ${COVERAGE_SRCS} PARENT_SCOPE)
    add_library(oslib loadfile.c graphics_dummy.c sound.c io.c text.c)
else()
    add_library(oslib loadfile.c graphics.c sound.c io.c text.c)
endif()

target_link_libraries(oslib glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
