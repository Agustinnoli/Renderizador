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


typedef struct linea {
punto_t p1;
punto_t p2;
} linea_t;


punto_t* copia;
linea_t* lineasADibujar;
int tamaniolineasADibujar;


void dibujarLinea(punto_t* p1, punto_t* p2) {

    float x1 = ((p1->x + 1) * ANCHO - PIXELSIZE)/2;
    float x2 = ((p2->x + 1) * ANCHO - PIXELSIZE)/2;
    float y1 = ((1 - p1->y) * ALTURA - PIXELSIZE)/2;
    float y2 = ((1 - p2->y) * ALTURA - PIXELSIZE)/2;
    SDL_Rect pixel;
    int xVectorDirector = (int) (x2 - x1);
    int yVectorDirector = (int) (y2 - y1);
    int CantPixelesADibujar;

    float xVectorDirector2 = - xVectorDirector;
    float yVectorDirector2 = - yVectorDirector;

    if(x1 > ANCHO){float mult = (ANCHO - x1)/xVectorDirector2; x1 = mult*xVectorDirector2 + x1;y1= mult*yVectorDirector2 + y1; }
    else if(x2 > ANCHO){float mult = (ANCHO - x2)/xVectorDirector; x2 = mult*xVectorDirector + x2;y2= mult*yVectorDirector + y2; }
    if(x1 < 0){float mult = (- x1)/xVectorDirector2; x1 = mult*xVectorDirector2 + x1;y1= mult*yVectorDirector2 + y1; }
    else if(x2 < 0){float mult = (- x2)/xVectorDirector; x2 = mult*xVectorDirector + x2;y2= mult*yVectorDirector + y2; }
    if(y1 > ALTURA){float mult = (ALTURA - y1)/yVectorDirector2; x1 = mult*xVectorDirector2 + x1;y1= mult*yVectorDirector2 + y1; }
    else if(y2 > ALTURA){float mult = (ALTURA - y2)/yVectorDirector; x2 = mult*xVectorDirector + x2;y2= mult*yVectorDirector + y2; }
    if(y1 < 0){float mult = (- y1)/yVectorDirector2; x1 = mult*xVectorDirector2 + x1;y1= mult*yVectorDirector2 + y1; }
    else if(y2 < 0){float mult = (- y2)/yVectorDirector; x2 = mult*xVectorDirector + x2;y2= mult*yVectorDirector + y2; }
    xVectorDirector = (int) (x2 - x1);
    yVectorDirector = (int) (y2 - y1);

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
    for(int i =0 ; i < tamaniolineasADibujar; i ++ ){
        lineasADibujar[i].p1.x = lineasADibujar[i].p1.x / lineasADibujar[i].p1.z;
        lineasADibujar[i].p1.y = lineasADibujar[i].p1.y / lineasADibujar[i].p1.z;
        lineasADibujar[i].p2.x = lineasADibujar[i].p2.x / lineasADibujar[i].p2.z;
        lineasADibujar[i].p2.y = lineasADibujar[i].p2.y / lineasADibujar[i].p2.z;

           
                  
    }

}
void dibujarPoligono(){
    for (int i = 0; i < tamaniolineasADibujar; i++){
        punto_t *p1 = &lineasADibujar[i].p1;
        punto_t *p2 = &lineasADibujar[i].p2;
        dibujarLinea(p1, p2);

    }
}
void clip3D(){
    tamaniolineasADibujar =0;
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){
        for (int j =0 ; j < 3; j ++){
            punto_t *p1 = &copia[poligonos[i][j]];
            punto_t *p2 = &copia[poligonos[i][(j +1)%3]];
            if(p1->z < NEARPLANE && p2->z < NEARPLANE){continue;}
            if(p1->z < NEARPLANE || p2->z < NEARPLANE){
                if(p1->z < NEARPLANE){punto_t* temp = p2; p2 = p1; p1 = temp;}
                punto_t vectorDirecotr = {p2->x-p1->x,p2->y-p1->y,p2->z-p1->z};
                float Intersecccion = (NEARPLANE- p2->z )/vectorDirecotr.z;
                punto_t puntoInterseccion ={vectorDirecotr.x*Intersecccion +p2->x,vectorDirecotr.y*Intersecccion +p2->y,vectorDirecotr.z*Intersecccion +p2->z};
                p2 = &puntoInterseccion;
            }
            bool p1Arriba = p1->y > p1->z;
            bool p1Abajo = p1->y < -p1->z;
            bool p1Derecha = p1->x > p1->z;
            bool p1Izquierda = p1->x < -p1->z;
            bool p2Arriba = p2->y > p2->z;
            bool p2Abajo = p2->y < -p2->z;
            bool p2Derecha = p2->x > p2->z;
            bool p2Izquierda = p2->x < -p2->z;
            if(p1Arriba&&p2Arriba){continue;}
            if(p1Abajo&&p2Abajo){continue;}
            if(p1Derecha&&p2Derecha){continue;}
            if(p1Izquierda&&p2Izquierda){continue;}
            lineasADibujar[tamaniolineasADibujar] = (linea_t) {*p1,*p2};
            tamaniolineasADibujar++;
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
    lineasADibujar = malloc((sizeof(linea_t)*sizeof(poligonos)*3)/sizeof(int[3]));
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
        clip3D();
        alejarTodos();
        
        dibujarPoligono();

        SDL_PumpEvents();
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16); //60FPS
        SDL_FillRect(surface, &fondo, COLOR_BLACK); 
    }
    free(copia);
    free(lineasADibujar);
    return 0;
}


