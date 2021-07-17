all: src/crisp.c
	cc -O3 -pedantic -ansi -std=c89 -o bin/crisp src/crisp.c $$(pkg-config --static --cflags --libs sdl2)
