#ifndef RENDER_H
#define RENDER_H
#define ANCHO 600
#define ALTO 600
#include <SDL2/SDL.h>

typedef struct punto3D {
    float x;
    float y;
    float z;
} punto3D_t;

typedef struct punto2D {
    int x;
    int y;
} punto2D_t;


typedef struct poligono3D {
    punto3D_t p1;
    punto3D_t p2;
    punto3D_t p3;
} poligono3D_t;

typedef struct poligono2D {
    punto2D_t p1;
    punto2D_t p2;
    punto2D_t p3;
} poligono2D_t;

typedef struct objeto {
    punto3D_t posicion;
    punto3D_t rotacion;
} objeto_t;

typedef struct modelo { 
    objeto_t objeto;
    punto3D_t *vertices;
    int (*poligonos)[3];
    size_t cantidadVertices;
    size_t cantidadPoligonos;

    punto3D_t (*boundingBox)[8]; 
} modelo_t;

typedef struct {
    objeto_t objeto;
    float intensidad;
} luz_t;

enum boundingBox {minXminYminZ,maxXminYminZ,minXmaxYminZ,minXminYmaxZ,maxXmaxYminZ,minXmaxYmaxZ,maxXminYmaxZ,maxXmaxYmaxZ,BoundingBoxSize};

typedef struct {
    punto3D_t posicion; 
    punto3D_t adelante; 
    punto3D_t arriba;       
    punto3D_t derecha;
} camara_t;

void renderInit(SDL_Surface* surface,SDL_Window* ventana,int cantLuces);
void renderUpdate();
void renderInput(const Uint8* teclado);
void renderDestroy();
void actualizarCamara(float deltaX, float deltaY);


#endif
