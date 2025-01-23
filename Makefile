PROGRAM=checkers
OBJECTS=checkers.o menu.o init-sdl.o scenegraph-gl.o texture.o
LDFLAGS=-lSDL2 -lGL -lGLU -lm
CFLAGS=	-Wall -W -O2

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
