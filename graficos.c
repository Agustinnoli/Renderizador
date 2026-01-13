#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "graficos.h"

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define ANCHO 600
#define ALTURA 600
#define PIXELSIZE 1
#define FPS 60
#define DELTATIEMPO 1.0f/FPS
#define PI 3.14159
SDL_Surface* surface;
SDL_Window* window;


typedef struct punto {
float x;
float y;
float z;

} punto_t;
punto_t* copia;

void dibujarLinea(punto_t* p1, punto_t* p2) {

    float x1 = ((p1->x + 1) * ANCHO - PIXELSIZE)/2;
    float x2 = ((p2->x + 1) * ANCHO - PIXELSIZE)/2;
    float y1 = ((1 - p1->y) * ALTURA - PIXELSIZE)/2;
    float y2 = ((1 - p2->y) * ALTURA - PIXELSIZE)/2;
    SDL_Rect pixel;
    int xVectorDirector = (int) (x2 - x1);
    int yVectorDirector = (int) (y2 - y1);
    int CantPixelesADibujar;
    if (abs(xVectorDirector) > abs(yVectorDirector)){ CantPixelesADibujar = abs(xVectorDirector);}else{ CantPixelesADibujar = abs(yVectorDirector);}
    if (CantPixelesADibujar == 0) { pixel = (SDL_Rect) {(int)x1, (int)y1, PIXELSIZE, PIXELSIZE};SDL_FillRect(surface, &pixel, COLOR_WHITE);return;}
    //la componente mas larga queda en 1, asi se puede sumar de a 1  
    float xNormalizado = (float) xVectorDirector / CantPixelesADibujar;
    float yNormalizado = (float) yVectorDirector / CantPixelesADibujar;
    for (int i = 0; i <= CantPixelesADibujar; i++) {
        pixel = (SDL_Rect){(int)x1, (int)y1, PIXELSIZE, PIXELSIZE};
        SDL_FillRect(surface, &pixel, COLOR_WHITE);
        x1 += xNormalizado;
        y1 += yNormalizado;
    }
}

punto_t puntos[] ={

{0.5,0.5,0.5},
{-0.5,0.5,0.5},
{-0.5,-0.5,0.5},
{0.5,-0.5,0.5},
{0.5,0.5,-0.5},
{-0.5,0.5,-0.5},
{-0.5,-0.5,-0.5},
{0.5,-0.5,-0.5}
};

int poligonos[][3] = {
{0,1,2},
{1,2,3}, 
{4,5,6},
{5,6,7}, 
{0,4,7},
{4,7,3}, 
{5,6,2},
{1,5,6}, 
{1,5,4},
{0,1,5}, 
{2,6,7},
{3,2,6}  
};


void ajustarZ(float dz){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
    copia[i].z += dz;
    }
}
void generarRotacionYPasarACopia(float angulo){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
        copia[i].x =puntos[i].x * cos(angulo) - puntos[i].z * sin(angulo);
        copia[i].y =puntos[i].y;
        copia[i].z =puntos[i].x * sin(angulo) + puntos[i].z * cos(angulo);;
    }
}
void alejarTodos(){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
        copia[i].x = copia[i].x/ copia[i].z;
        copia[i].y = copia[i].y/ copia[i].z;
    }

}
void dibujarPoligono(){
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){
        punto_t *p1 = &copia[poligonos[i][0]];
        punto_t *p2 = &copia[poligonos[i][1]];
        punto_t *p3 = &copia[poligonos[i][2]];
        dibujarLinea(p1, p2);
        dibujarLinea(p2, p3);
        dibujarLinea(p3, p1);
    }
}
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,ANCHO , ALTURA, 0);
    surface = SDL_GetWindowSurface(window);
    SDL_Rect fondo = (SDL_Rect) {0, 0, 900, 600};
    bool running = true;
    SDL_Event event;
    float angulo = 0;
    float dz = 2;
    copia = malloc(sizeof(puntos));
    while(running){
        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
            }
            else if (event.type == SDL_MOUSEMOTION && event.motion.state != 0){
                //rect.x = event.motion.x - 100;
                //rect.y = event.motion.y - 100;
        
            }
            if (event.type == SDL_KEYDOWN ){
                if(event.key.keysym.sym == SDLK_UP){dz -= DELTATIEMPO;}
                if(event.key.keysym.sym == SDLK_DOWN){dz += DELTATIEMPO;}
                if(event.key.keysym.sym == SDLK_LEFT){angulo += PI*DELTATIEMPO;}
                if(event.key.keysym.sym == SDLK_RIGHT){angulo -= PI*DELTATIEMPO;}
        
            }
        }
        generarRotacionYPasarACopia(angulo);
        ajustarZ(dz);
        alejarTodos();
        dibujarPoligono();

        SDL_UpdateWindowSurface(window);
        SDL_Delay(16); //60FPS
        SDL_FillRect(surface, &fondo, COLOR_BLACK); 
    }
    free(copia);
    return 0;
}


