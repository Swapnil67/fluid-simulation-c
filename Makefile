PKGS=sdl2
CFLAGS=-Wall -Wextra -Wconversion -pedantic `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --libs $(PKGS)`

fluid: fluid.c
	$(CC) $(CFLAGS) fluid.c -o fluid $(LIBS)