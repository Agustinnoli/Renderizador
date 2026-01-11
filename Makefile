all:
	gcc -o renderer graficos.c -g `sdl2-config --cflags --libs`

clean:
	rm -f renderer