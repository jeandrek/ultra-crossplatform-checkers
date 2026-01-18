PROGRAM=checkers
OBJECTS=checkers.o game.o game_display.o game_interaction.o game_checkers.o main_menu.o main-x11.o sprite.o scenegraph-gl.o texture.o input.o text.o
LDFLAGS=-lX11 -lGL -lGLU -lm
CFLAGS=	-Wall -W -Og -g

$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(OBJECTS)
