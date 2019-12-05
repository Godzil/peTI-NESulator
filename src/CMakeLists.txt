#
# peTI-NESulator CMake
#
# Created by Manoël Trapier.
# Copyright (c) 2002-2019 986-Studio.
#
##########################
# Configurations variables
##########################
set(PPU_ISPAL OFF CACHE BOOL "When switch to ON the PPU is in PAL mode, else it will act as a NTSC one.")
set(Q6502_DEBUGGER OFF CACHE BOOL "Activate the Quick6502 debugger?")
set(USE_SOUND OFF CACHE BOOL "Activate the sound?")
set(DETECT_BUS_CONFLICT OFF CACHE BOOL "Activate the bus conflit detector? (Could slow down the emulator a lot.)")

set(USE_EFENCE OFF CACHE BOOL "Use electricfence memory debugger?")
set(USE_PROFILING OFF CACHE BOOL "Use profiling tools? (Will slow down a lot.)")

set(USE_ALLEGRO ON CACHE BOOL "Use Allegro backend")

option(COVERALLS "Generate coveralls data" OFF)

##########################
# Link & Compile flags
##########################

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Werror ${PLATFORM_FLAGS}")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Werror ${PLATFORM_FLAGS}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/external/coveralls-cmake/cmake)

add_executable(petines main.c paddle.c NESCarts.c)

add_definitions(-DNO_DECIMAL)

if (PPU_ISPAL)
    add_definitions(-DISPAL)
else (PPU_ISPAL)
    add_definitions(-DISNTSC)
endif (PPU_ISPAL)

if (Q6502_DEBUGGER)
    add_definitions(-DDEBUG)
endif (Q6502_DEBUGGER)

if (USE_SOUND)
    add_definitions(-DUSE_SOUND)
endif (USE_SOUND)

if (DETECT_BUS_CONFLICT)
    add_definitions(-DDETECT_BUS_CONFLICT)
endif (DETECT_BUS_CONFLICT)

if (USE_PROFILING)
    if (CMAKE_BUILD_TYPE MATCHES Rel)
        SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Debug info is forced" FORCE)
    else (CMAKE_BUILD_TYPE MATCHES Rel)
        SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Debug info is forced" FORCE)
    endif (CMAKE_BUILD_TYPE MATCHES Rel)

    set(CMAKE_C_FLAGS -pg)
endif (USE_PROFILING)

if (COVERALLS)
    add_definitions(-DRUN_COVERAGE)
    include(Coveralls)
    coveralls_turn_on_coverage()
endif ()

include_directories(include)

add_subdirectory(apu)
add_subdirectory(corecpu)
add_subdirectory(mappersmanager)
add_subdirectory(memorymanager)
add_subdirectory(pluginsmanager)
add_subdirectory(ppu)


if (TARGET_TI68k)
    add_subdirectory(os/ti68k)
elseif (WIN32)
    add_subdirectory(os/win32)
else (TARGET_TI68k)
    add_subdirectory(os/unix)
endif (TARGET_TI68k)

#find_library(PTHREADLIB pthread)

if (COVERALLS)
    set(COVERAGE_SRCS src/main.c src/paddle.c src/NESCarts.c ${COVERAGE_SRCS})

    # Create the coveralls target.
    coveralls_setup(
            "${COVERAGE_SRCS}" # The source files.
            ON                 # If we should upload.
    ) # (Optional) Alternate project cmake module path.
endif ()

target_link_libraries(petines apu corecpu mappermanager memorymanager pluginsmanager ppu oslib)# ${PTHREADLIB})

add_test(NAME petines_test_1 COMMAND $<TARGET_FILE:petines> ${PROJECT_SOURCE_DIR}/data/bad_apple_2.nes)
add_test(NAME petines_test_2 COMMAND $<TARGET_FILE:petines> ${PROJECT_SOURCE_DIR}/data/trollburner_demo.nes)
