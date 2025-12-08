PROGRAM=checkers
OBJECTS=checkers.o game.o game_display.o game_interaction.o game_checkers.o menu.o init-sdl.o sprite.o scenegraph-gl.o texture.o input.o
LDFLAGS=-lSDL2 -lGL -lGLU -lm
CFLAGS=	-Wall -W -Og -g

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
