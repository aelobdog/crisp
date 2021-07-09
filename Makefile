debug: src/*.c
	cc -Wall -Wextra -pedantic -ansi -std=c89 -o bin/crisp_debug src/*.c $$(pkg-config --static --libs glfw3) -lGL -lGLU

release: src/*.c
	cc -O3 -pedantic -ansi -std=c89 -o bin/crisp_release src/*.c $$(pkg-config --static --libs glfw3) -lGL -lGLU

clean:
	rm bin/*
