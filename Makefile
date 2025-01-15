PROGRAM=checkers
OBJECTS=checkers.o scenegraph-gl.o texture.o
LDFLAGS=-lSDL2 -lGL -lGLU
CFLAGS=	-Wall -W -O2

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
