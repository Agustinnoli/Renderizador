#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define ANCHO 600
#define ALTURA 600
#define PIXELSIZE 2
SDL_Surface* surface;
SDL_Window* window;


typedef struct punto {
float x;
float y;
float z;

} punto_t;




/*
typedef struct linea {
int x1;
int y1;
int x2;
int y2;
} linea_t;
*/
void dibujarLinea(punto_t* p1, punto_t* p2) {

    float x1 = (p1->x +1) * ANCHO/2;
    float x2 = (p2->x +1) * ANCHO/2;
    float y1 = (ALTURA/2)* (1-p1->y);
    float y2 = (ALTURA/2) * (1-p2->y);
    SDL_Rect pixel;


    int xVectorDirector = (int) (x2 - x1);
    int yVectorDirector = (int) (y2 - y1);
    int CantPixelesADibujar;
    if (abs(xVectorDirector) > abs(yVectorDirector)){ CantPixelesADibujar = abs(xVectorDirector);}else{ CantPixelesADibujar = abs(yVectorDirector);}
    if (CantPixelesADibujar == 0) { pixel = (SDL_Rect) {(int)x1, (int)y1, PIXELSIZE, PIXELSIZE};SDL_FillRect(surface, &pixel, COLOR_WHITE);return;}
    //la componente mas larga queda en 1, asi se puede sumar de a 1  
    float xNormalizado = xVectorDirector / CantPixelesADibujar;
    float yNormalizado = yVectorDirector / CantPixelesADibujar;
    for (int i = 0; i <= CantPixelesADibujar; i++) {
        pixel = (SDL_Rect){(int)x1, (int)y1, PIXELSIZE, PIXELSIZE};
        SDL_FillRect(surface, &pixel, COLOR_WHITE);
        x1 += xNormalizado;
        y1 += yNormalizado;
    }
}


int main(){
    //setenv("SDL_VIDEODRIVER", "wayland", 1);
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ALTURA, ANCHO, 0);
    surface = SDL_GetWindowSurface(window);
    SDL_Rect rect = (SDL_Rect) {200, 200, 200, 200};
    SDL_Rect fondo = (SDL_Rect) {0, 0, 900, 600};
    bool running = true;
    SDL_Event event;
    while(running){
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
            }
            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0){
                rect.x = event.motion.x - 100;
                rect.y = event.motion.y - 100;
        
            }
        //SDL_FillRect(surface, &fondo, COLOR_BLACK);    
        //SDL_FillRect(surface, &rect, COLOR_WHITE);
        //linea_t* linea = crearLinea(1,2,10,20);
        punto_t p1 = (punto_t){0,0,0};
        punto_t p2 = (punto_t){0,0,0};
        dibujarLinea(&p1,&p2);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);

        }
    }
    return 0;
}

/*
void dibujarLineaConPendiente(linea_t* lineaP, SDL_Surface *surface){
    linea_t linea = *lineaP;
    int pendiente = (linea.y2 - linea.y1)/(linea.x2- linea.x1);
    int b = linea.y1/(pendiente* linea.x1);
    SDL_Rect pixel;
    int y = linea.x1 * pendiente + b;
    for(int x = linea.x1;x<= linea.x2 || y <= linea.y2; x++ ){ // no puedo poner simplementa x1<x2 por las lienas verticales
    pixel = (SDL_Rect) {x, y, 1,1};    
    SDL_FillRect(surface, &pixel, COLOR_WHITE);
    y = x * pendiente + b;    
    }
}
*/ 
