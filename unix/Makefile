CC = gcc

#For Debug
#CFLAGS = -g `allegro-config --cflags debug` -I../src -Wall -pg -DISPAL -DNO_DECIMAL -DUNIX -DFAST_RDOP -DLSB_FIRST -DUSE_SOUND

#For Retail
CFLAGS = -I$(SRC)/include -I$(SRC) -g `allegro-config --cflags release`l -DISNTSC -DNO_DECIMAL -DUNIX -DFAST_RDOP -DLSB_FIRST -DDEBUG -DUSE_SOUND
# -fomit-frame-pointer -funroll-loops 
#-DNO_N_KEY
#-DUSE_SOUND
#-DDETECT_BUS_CONFLICT
#-DISNTSC or -DISPAL
#-DDEBUG 

LDFLAGS = `allegro-config --libs debug` -lpthread
#-pg #-lefence
#debug` -lpthread -lefence -pg
#release` -lpthread
#-pg

SRC=../src
PLUGINS_DIR=pluginsmanager/plugins/
MAPPERS_DIR=mappersmanager/mappers/

PLUGINS=$(shell ls $(SRC)/$(PLUGINS_DIR)/*.c)
MAPPERS=$(shell ls $(SRC)/$(MAPPERS_DIR)/*.c)

all: tines

tines: tines.a corecpu.a apu.a ppu.a plugins.a mappers.a memory.a
	$(CC)  -o $@ $(LDFLAGS) $^

apu.a: $(SRC)/apu/Sound.c $(SRC)/apu/SndAlleg.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
corecpu.a: $(SRC)/corecpu/Debug.c $(SRC)/corecpu/M6502.c
	$(CC) $(CFLAGS) -Wno-pointer-sign -c $^ -o $@
	
tines.a: $(SRC)/main.c $(SRC)/paddle.c $(SRC)/NESCarts.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
memory.a: $(SRC)/memorymanager/memory.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
ppu.a: $(SRC)/ppu/ppu.c $(SRC)/ppu/ppu.memory.c $(SRC)/ppu/debug/ppu.debug.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
mappers.a: $(SRC)/mappersmanager/manager.c $(SRC)/mappersmanager/utils.c $(MAPPERS)
	$(CC) $(CFLAGS) -c $^ -o $@
	
plugins.a: $(SRC)/pluginsmanager/manager.c $(PLUGINS)
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -Rf *.o *~ core