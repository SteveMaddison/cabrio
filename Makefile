CC=gcc
BIN_DIR=/usr/local/bin
DATA_DIR=/usr/local/share/cabrio
CFLAGS=-g -Wall -DDATA_DIR=\"$(DATA_DIR)\"
LDFLAGS=-lSDL_image -lSDL_gfx -lglut -lSDL_ttf -lSDL_mixer -lxml2
INCLUDES=-I./include -I/usr/include/libxml2

INSTALL=/usr/bin/install -c

cabrio: main.o ogl.o sdl_wrapper.o config.o bg.o menu.o game_sel.o \
	game.o font.o hint.o platform.o submenu.o \
	sound.o event.o key.o control.o setup.o sdl_ogl.o \
	category.o focus.o emulator.o snap.o image.o
	$(CC) -o $@ $(LDFLAGS) $^

.c.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

install: cabrio
	$(INSTALL) -m 755 -d $(DATA_DIR)/fonts $(DATA_DIR)/pixmaps $(DATA_DIR)/sounds
	$(INSTALL) -m 644 -t $(DATA_DIR)/fonts data/fonts/*
	$(INSTALL) -m 644 -t $(DATA_DIR)/pixmaps data/pixmaps/*
	$(INSTALL) -m 644 -t $(DATA_DIR)/sounds data/sounds/*	
	$(INSTALL) -m 755 -d $(BIN_DIR)
	$(INSTALL) -m 755 -t $(BIN_DIR) cabrio

clean:
	rm -f cabrio *.o core

