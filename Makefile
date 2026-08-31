PREFIX=		/usr/local
DATADIR=	$(PREFIX)/share/games/checkers

PROGRAM=	checkers
OBJECTS=	checkers.o game.o game_display.o game_interaction.o game_checkers.o menu.o net_menu.o main-x11.o sprite.o scenegraph-gl.o texture.o input.o text.o gui.o text_input.o game_net.o game_computer.o game_save.o select_file.o model.o asset.o error.o
CFLAGS+=	-Wall -O2 -g -DDATADIR="\"$(DATADIR)\"" $(CFLAGS_DNS_SD$(USE_DNS_SD))
CFLAGS_DNS_SD1=-DUSE_DNS_SD
CFLAGS+=	-DUSE_X11
LDFLAGS+=	-lX11 -lGL -lGLU -lm -pthread $(LIBS_DNS_SD$(USE_DNS_SD))
LIBS_DNS_SD1=	-ldns_sd

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)

install: $(PROGRAM)
	mkdir -p $(PREFIX)/games
	mkdir -p $(DATADIR)/assets/textures
	mkdir -p $(DATADIR)/assets/models
	install -m 755 $(PROGRAM) $(PREFIX)/games
	install -m 644 assets/textures/* $(DATADIR)/assets/textures
	install -m 644 assets/models/* $(DATADIR)/assets/models
