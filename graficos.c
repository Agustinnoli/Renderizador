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
typedef struct poligono {
punto_t p1;
punto_t p2;
punto_t p3;
} poligono_t;

punto_t* copia;
//linea_t* lineasADibujar;
poligono_t* poligonosADibujar;

int tamanioPoligonosADibujar;


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
    // s epuede simplificar haciendo primero que 1p sea el punto mas a la dereha y hacer la logica, y por un cambio de variables hacer luego que la var de p1 sea el mmas arriba 
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
    for(int i =0 ; i < tamanioPoligonosADibujar; i ++ ){
        poligonosADibujar[i].p1.x = poligonosADibujar[i].p1.x / poligonosADibujar[i].p1.z;
        poligonosADibujar[i].p1.y = poligonosADibujar[i].p1.y / poligonosADibujar[i].p1.z;
        poligonosADibujar[i].p2.x = poligonosADibujar[i].p2.x / poligonosADibujar[i].p2.z;
        poligonosADibujar[i].p2.y = poligonosADibujar[i].p2.y / poligonosADibujar[i].p2.z;
        poligonosADibujar[i].p3.x = poligonosADibujar[i].p3.x / poligonosADibujar[i].p3.z;
        poligonosADibujar[i].p3.y = poligonosADibujar[i].p3.y / poligonosADibujar[i].p3.z;
           
                  
    }

}
void dibujarPoligono(){
    for (int i = 0; i < tamanioPoligonosADibujar; i++){
        punto_t *p1 = &poligonosADibujar[i].p1;
        punto_t *p2 = &poligonosADibujar[i].p2;
        punto_t *p3 = &poligonosADibujar[i].p3;
        dibujarLinea(p1, p2);
        dibujarLinea(p1, p3);
        dibujarLinea(p3, p2);

    }
}

punto_t jijona(punto_t* dentro, punto_t* afuera){
        punto_t vectorDirecotr = {afuera->x-dentro->x,afuera->y-dentro->y,afuera->z-dentro->z};
        float Intersecccion = (NEARPLANE- afuera->z )/vectorDirecotr.z;
        //afuera->x = vectorDirecotr.x*Intersecccion +afuera->x;
        //afuera->y =vectorDirecotr.y*Intersecccion +afuera->y;
        //afuera->z = NEARPLANE;
        return (punto_t) {vectorDirecotr.x*Intersecccion +afuera->x,vectorDirecotr.y*Intersecccion +afuera->y,NEARPLANE};
        
}
bool checkeoFueraPantallaPoligono3D(punto_t* p1,punto_t* p2,punto_t* p3){
    bool p1Arriba = p1->y > p1->z;
    bool p1Abajo = p1->y < -p1->z;
    bool p1Derecha = p1->x > p1->z;
    bool p1Izquierda = p1->x < -p1->z;
    bool p2Arriba = p2->y > p2->z;
    bool p2Abajo = p2->y < -p2->z;
    bool p2Derecha = p2->x > p2->z;
    bool p2Izquierda = p2->x < -p2->z;
    bool p3Arriba = p3->y > p3->z;
    bool p3Abajo = p3->y < -p3->z;
    bool p3Derecha = p3->x >  p3->z;
    bool p3Izquierda = p3->x < -p3->z;
    return(p1Arriba&&p2Arriba&&p3Arriba)||(p1Abajo&&p2Abajo&&p3Abajo)||(p1Derecha&&p2Derecha&&p3Derecha)||(p1Izquierda&&p2Izquierda&&p3Izquierda);    
}
void clip3D(){
    tamanioPoligonosADibujar =0;
    int yay;
    punto_t* temp;
    punto_t temp2;
    punto_t temp3;
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){

            punto_t *p1 = &copia[poligonos[i][0]];
            punto_t *p2 = &copia[poligonos[i][1]];
            punto_t *p3 = &copia[poligonos[i][2]];
            yay = 0;
            if(p1->z < NEARPLANE){ yay |= 0b001;}
            if(p2->z < NEARPLANE){ yay |= 0b010;}// esto podria ser un enum 
            if(p3->z < NEARPLANE){ yay |= 0b100;}
            switch (yay){
                case 0b000://ninguno afuera
                    if(checkeoFueraPantallaPoligono3D(p1,p2,p3)){continue;}
                    poligonosADibujar[tamanioPoligonosADibujar] = (poligono_t) {*p1,*p2,*p3};
                    tamanioPoligonosADibujar++;
                continue;
                break;
                case 0b001://p1 afuera
                    goto caso1Afuera;
                break;
                case 0b010://p2 afuera
                    temp = p2; p2 = p1; p1 = temp;
                    goto caso1Afuera;
                break;
                case 0b100://p3 afuera
                    temp = p3; p3 = p1; p1 = temp;
                    goto caso1Afuera;
                break;
                case 0b011://p1 p2 afuera
                    goto caso2Afuera;
                break;
                case 0b101://p1 p3 afuera
                    temp = p3; p3 = p2; p2 = temp;
                    goto caso2Afuera;
                break;
                case 0b110://p2 p3 afuera
                    temp = p3; p3 = p1; p1 = temp;
                    goto caso2Afuera;
                break;
                case 0b111://p1 p2 p3 afuera
                    continue;
                break;
            }
            caso1Afuera: // asumo que p1 esta afuera
                temp2 = jijona(p3,p1);
                if(!checkeoFueraPantallaPoligono3D(&temp2,p2,p3)){
                    poligonosADibujar[tamanioPoligonosADibujar] = (poligono_t) {temp2,*p2,*p3};
                    tamanioPoligonosADibujar++;
                }
                temp3 = jijona(p2,p1);
                if(checkeoFueraPantallaPoligono3D(&temp3,p2,&temp2)){continue;}
                poligonosADibujar[tamanioPoligonosADibujar] = (poligono_t) {temp3,*p2,temp2};
                tamanioPoligonosADibujar++;
                continue; 

            caso2Afuera: // asumo p1 p2 afuera;
                temp2 = jijona(p3,p1);
                temp3 = jijona(p3,p2);
                if(checkeoFueraPantallaPoligono3D(&temp3,&temp2,p3)){continue;}
                poligonosADibujar[tamanioPoligonosADibujar] = (poligono_t) {temp3,temp2,*p3};
                tamanioPoligonosADibujar++;
                continue;
        
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
    //lineasADibujar = malloc((sizeof(linea_t)*sizeof(poligonos)*3)/sizeof(int[3]));
    poligonosADibujar = malloc((sizeof(poligono_t)*sizeof(poligonos)*2)/sizeof(int[3]));
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

        if (teclado[SDL_SCANCODE_W]){dz -= DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_S]){dz += DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_A]){dx += DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_D]){dx -= DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_SPACE]){dy -= DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_LSHIFT]){dy += DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_LEFT]){angulo += PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_RIGHT]){angulo -= PI*DELTATIEMPO;}

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
    //free(lineasADibujar);
    free(poligonosADibujar);
    return 0;
}

