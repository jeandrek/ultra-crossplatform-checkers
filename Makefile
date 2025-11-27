PROGRAM=checkers
OBJECTS=checkers.o game.o game_display.o menu.o init-sdl.o scenegraph-gl.o texture.o
LDFLAGS=-lSDL2 -lGL -lGLU -lm
CFLAGS=	-Wall -W -Og -g

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
