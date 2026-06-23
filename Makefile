all:
	gcc -o render main.c render.c parser.c -g -lm `sdl2-config --cflags --libs`
clean:
	rm -f renderer