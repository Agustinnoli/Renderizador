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
#define NEARPLANE 0.1f
SDL_Surface* surface;
SDL_Window* window;
float angulo = 0;
float dz = 2;
float dx = 0;
float dy = 0;

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
    if(x1 <= 0 || x1 > ANCHO ||y1 <= 0 || y1 > ALTURA){ x1 = x2; y1 = y2; xNormalizado = -xNormalizado;yNormalizado = -yNormalizado;}
    for (int i = 0; i <= CantPixelesADibujar; i++) {
        pixel = (SDL_Rect){(int)x1, (int)y1, PIXELSIZE, PIXELSIZE};
        SDL_FillRect(surface, &pixel, COLOR_WHITE);
        x1 += xNormalizado;
        y1 += yNormalizado;
        if(x1 < 0 || x1 > ANCHO ||y1 < 0 || y1 > ALTURA){ return;}// esto consume mucoh, deberia de calcular la intersecccion con el plano para saber hasta que punto deberia de  dibujarr
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

void ajustar(){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
        copia[i].x += dx;
        copia[i].y += dy;
        copia[i].z += dz;
    }
}
void generarRotacionYPasarACopia(){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
        copia[i].x =puntos[i].x * cos(angulo) - puntos[i].z * sin(angulo);
        copia[i].y =puntos[i].y;
        copia[i].z =puntos[i].x * sin(angulo) + puntos[i].z * cos(angulo);
        //copia[i].x =puntos[i].x * cos(angulo) - puntos[i].y * sin(angulo);
        //copia[i].y =puntos[i].x * sin(angulo) + puntos[i].y * cos(angulo);
        //copia[i].z =puntos[i].z;
        //copia[i].x =puntos[i].x;
        //copia[i].y =puntos[i].y * cos(angulo) - puntos[i].z * sin(angulo);
        //copia[i].z =puntos[i].y * sin(angulo) + puntos[i].z * cos(angulo);
    }
}
void alejarTodos(){
    for(int i =0 ; i < sizeof(puntos)/sizeof(punto_t); i ++ ){
        //if (copia[i].z<=NEARPLANE && copia[i].z>=0){copia[i].z = NEARPLANE;}
        //if (copia[i].z>=-NEARPLANE && copia[i].z<=0){copia[i].z = -NEARPLANE;}
        copia[i].x = copia[i].x/ copia[i].z;
        copia[i].y = copia[i].y/ copia[i].z;
           
                  
    }

}
void dibujarPoligono(){
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){
        punto_t *p1 = &copia[poligonos[i][0]];
        punto_t *p2 = &copia[poligonos[i][1]];
        punto_t *p3 = &copia[poligonos[i][2]];
        if(!(p1->z < 0.1 && p2->z < 0.1 && p3->z < 0.1) ){ // funciona con lo que debe de hacer pero au asi falta porner los intermedio tipo si un punto quedo afuera y los otros 2 adentro 
            dibujarLinea(p1, p2);
            dibujarLinea(p2, p3);
            dibujarLinea(p3, p1);
        }
    }
}
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,ANCHO , ALTURA, 0);
    surface = SDL_GetWindowSurface(window);
    SDL_Rect fondo = (SDL_Rect) {0, 0, ANCHO, ALTURA};
    bool running = true;
    SDL_Event event;
    const Uint8 *teclado = SDL_GetKeyboardState(NULL);
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
        }

        if (teclado[SDL_SCANCODE_W]) dz -= DELTATIEMPO;
        if (teclado[SDL_SCANCODE_S]) dz += DELTATIEMPO;
        if (teclado[SDL_SCANCODE_A]) dx += DELTATIEMPO;
        if (teclado[SDL_SCANCODE_D]) dx -= DELTATIEMPO;
        if (teclado[SDL_SCANCODE_SPACE]) dy -= DELTATIEMPO;
        if (teclado[SDL_SCANCODE_LSHIFT]) dy += DELTATIEMPO;
        if (teclado[SDL_SCANCODE_LEFT]) angulo += PI*DELTATIEMPO;
        if (teclado[SDL_SCANCODE_RIGHT]) angulo -= PI*DELTATIEMPO;

        generarRotacionYPasarACopia();
        ajustar();
        alejarTodos();
        dibujarPoligono();

        SDL_PumpEvents();
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16); //60FPS
        SDL_FillRect(surface, &fondo, COLOR_BLACK); 
    }
    free(copia);
    return 0;
}


