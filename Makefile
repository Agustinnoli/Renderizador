all:
	gcc -o renderer graficos.c -g -lm `sdl2-config --cflags --libs`

clean:
	rm -f renderer