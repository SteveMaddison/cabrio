CC=gcc
BIN_DIR=/usr/bin
DATA_DIR=/usr/share/cabrio
OPTIMIZE=-O3 -ffast-math
ifeq ($(DEBUG), 1)
   OPTIMIZE=-O0
endif
CFLAGS+=-g -Wall $(OPTIMIZE) -DDATA_DIR=\"$(DATA_DIR)\"
LDFLAGS+=-lGL -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_ttf -lSDL2_mixer -lGLU -lxml2
INCLUDES=-I./include -I/usr/include/libxml2

# Works with libav 9.18 and 10.7 (buggy with this, weird behavior)
ifneq (,$(BUNDLED_LIBAV))
	LIBAV_PATH=libav
	LDFLAGS+=-L./$(LIBAV_PATH)/linux/lib -lavformat -lavcodec -lavutil -lswscale -lm -lz -lpthread
	INCLUDES+=-I./$(LIBAV_PATH)/linux/include
	ifeq (,$(wildcard ./$(LIBAV_PATH)/linux/lib/libavformat.a))
		TMP:=$(shell cp -f ./build_libav_linux.sh ./$(LIBAV_PATH)/ && cd $(LIBAV_PATH)/ && ./build_libav_linux.sh)
	endif
else
	LDFLAGS+=-lavutil -lavformat -lavcodec -lswscale
endif

INSTALL=/usr/bin/install -c

cabrio: main.o ogl.o sdl_wrapper.o config.o bg.o menu.o game_sel.o \
	game.o font.o hint.o platform.o submenu.o \
	sound.o event.o key.o control.o setup.o sdl_ogl.o video.o packet.o frame.o \
	category.o focus.o emulator.o snap.o media.o location.o lookup.o
	$(CC) -o $@ $^ $(LDFLAGS)

.c.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

install: cabrio
	$(INSTALL) -m 755 -d $(DATA_DIR)/fonts $(DATA_DIR)/pixmaps $(DATA_DIR)/sounds
	$(INSTALL) -m 644 -t $(DATA_DIR)/fonts data/fonts/*
	$(INSTALL) -m 644 -t $(DATA_DIR)/pixmaps data/pixmaps/*
	$(INSTALL) -m 644 -t $(DATA_DIR)/sounds data/sounds/*	
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes/lmc
	$(INSTALL) -m 644 -t $(DATA_DIR)/themes/lmc data/themes/lmc/*
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes/carousel
	$(INSTALL) -m 644 -t $(DATA_DIR)/themes/carousel data/themes/carousel/*
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes/ice
	$(INSTALL) -m 644 -t $(DATA_DIR)/themes/ice data/themes/ice/*
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes/industrial
	$(INSTALL) -m 644 -t $(DATA_DIR)/themes/industrial data/themes/industrial/*
	$(INSTALL) -m 755 -d $(DATA_DIR)/themes/wood
	$(INSTALL) -m 644 -t $(DATA_DIR)/themes/wood data/themes/wood/*
	$(INSTALL) -m 644 -t $(DATA_DIR)/../applications/ cabrio.desktop
	$(INSTALL) -m 755 -d $(BIN_DIR)
	$(INSTALL) -m 755 -t $(BIN_DIR) cabrio

deb:
	debuild -i -us -uc -b

debclean:
	debuild clean

clean:
	rm -f cabrio *.o core core.*

