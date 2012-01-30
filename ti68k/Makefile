INCLUDE_FILES = ../src/include

APU_FILES = ../src/apu/Sound.c
CORECPU_FILES = ../src/corecpu/corecpu.c
MAPPERS_FILES = ../src/mappersmanager/manager.c ../src/mappersmanager/mappers/aorom.c ../src/mappersmanager/mappers/cnrom.c ../src/mappersmanager/mappers/iremh3001.c ../src/mappersmanager/mappers/mmc1.c ../src/mappersmanager/mappers/mmc3.c ../src/mappersmanager/mappers/mmc4.c ../src/mappersmanager/mappers/norom.c ../src/mappersmanager/mappers/unrom.c
MEMORY_FILES = ../src/memorymanager/memory.c 
PLUGINS_FILES = ../src/pluginsmanager/manager.c
PPU_FILES = ../src/ppu/ppu.c

C_FILES = ../src/os/ti68k/main.c ../src/main.c ../src/NESCarts.c ../src/paddle.c ${APU_FILES} ${CORECPU_FILES} ${MAPPERS_FILES} ${MEMORY_FILES} ${PLUGINS_FILES} ${PPU_FILES}


tines: ${C_FILES}
	gtc -I ${INCLUDE_FILES} -o $@ ${C_FILES}