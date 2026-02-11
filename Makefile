PROGRAM=	checkers
OBJECTS=	checkers.o game.o game_display.o game_interaction.o game_checkers.o menu.o net_menu.o main-x11.o sprite.o scenegraph-gl.o texture.o input.o text.o gui.o text_input.o game_net.o
CFLAGS+=	-Wall -Og -g $(CFLAGS_BONJOUR$(USE_BONJOUR))
CFLAGS_BONJOUR1=-DUSE_BONJOUR
CFLAGS+=	-DUSE_X11
LDFLAGS+=	-lX11 -lGL -lGLU -lm $(LIBS_BONJOUR$(USE_BONJOUR))
LIBS_BONJOUR1=	-ldns_sd

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
