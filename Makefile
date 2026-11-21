PREFIX=		/usr/local
DATADIR=	$(PREFIX)/share/games/checkers

PROGRAM=	checkers
OBJECTS=	checkers.o game.o game_display.o game_interaction.o game_checkers.o menu.o net_menu.o main-x11.o sprite.o scenegraph-gl.o scenegraph_shaders.o texture.o input.o text.o gui.o text_input.o game_net.o game_computer.o game_save.o select_file.o model.o asset.o error.o
CFLAGS+=	-Wall -O2 -std=gnu99 -DDATADIR="\"$(DATADIR)\""
CFLAGS+=	$(CFLAGS_DNS_SD$(USE_DNS_SD))
CFLAGS_DNS_SD1=-DUSE_DNS_SD
CFLAGS+=	-DUSE_X11
CFLAGS+=	$(CFLAGS_SHADERS$(USE_SHADERS))
CFLAGS_SHADERS1=-DUSE_SHADERS
LDFLAGS+=	-lX11 -lGL -lGLU -lm -pthread $(LIBS_DNS_SD$(USE_DNS_SD))
LIBS_DNS_SD1=	-ldns_sd

USE_SHADERS=	1

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

obj2model: obj2model.o
	$(CC) -o $@ obj2model.o -ll

clean:
	rm -f $(PROGRAM) $(OBJECTS) obj2model

install: $(PROGRAM)
	mkdir -p $(PREFIX)/games
	mkdir -p $(DATADIR)/assets/textures
	mkdir -p $(DATADIR)/assets/models
	install -m 755 $(PROGRAM) $(PREFIX)/games
	install -m 644 assets/textures/* $(DATADIR)/assets/textures
	install -m 644 assets/models/* $(DATADIR)/assets/models
