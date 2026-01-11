all:
	gcc -o renderer graficos.c `sdl2-config --cflags --libs`

clean:
	rm -f renderer