#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "parser.h"


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
float dz = 2;
float dx = 0;
float dy = 0;

modelo_t* modeloSeleccionado;
modelo_t* modelos;
size_t modelosSize;

punto3D_t* VerticesTransformados;

poligono3D_t* poligonosClipeados;
size_t poligonosClipeadosSize;

poligono2D_t* poligonosADibujar;

size_t anchoRealMemoria;
float* zbuffer;

void transformarVertices(modelo_t* modelo){
    punto3D_t *vertices = modelo->vertices; 
    size_t cantidadVertices = modelo->cantidadVertices;

    //rotar
    float seno = sin(modelo->rotacion);
    float coseno = cos(modelo->rotacion);
    for(int i =0 ; i < cantidadVertices; i ++){
        VerticesTransformados[i].x = vertices[i].x * coseno - vertices[i].z * seno;
        VerticesTransformados[i].y = vertices[i].y;
        VerticesTransformados[i].z = vertices[i].x * seno + vertices[i].z * coseno;
    }
    //trasladar
    punto3D_t posicion =modelo->posicion;
    for(int i =0 ; i < cantidadVertices; i ++ ){
        VerticesTransformados[i].x += dx + posicion.x;
        VerticesTransformados[i].y += dy + posicion.y;
        VerticesTransformados[i].z += dz + posicion.z;  
    }
}




punto2D_t coordsAPantalla(float x, float y){
    return (punto2D_t) {(int)((x + 1) * ANCHO - PIXELSIZE)/2,(int) ((1 - y) * ALTURA - PIXELSIZE)/2} ;  
}
void proyectar2D(){

    for(int i =0 ; i < poligonosClipeadosSize; i ++ ){
        punto2D_t* dibujar = (punto2D_t*) &poligonosADibujar[i].p1;
        punto3D_t* clipeado = (punto3D_t*)&poligonosClipeados[i];
        for (int j = 0; j < 3; j++) {
            float x =  clipeado[j].x / clipeado[j].z;
            float y =  clipeado[j].y / clipeado[j].z;
            dibujar[j] = coordsAPantalla(x,y);
        }
    }
}


void dibujarPoligono(){
    if (SDL_LockSurface(surface) != 0) {return;}
    size_t ancho_real_memoria = surface->pitch / 4;
    Uint32 *pixels = (Uint32 *)surface->pixels;
    srand(0); 
    for (int i = 0; i < poligonosClipeadosSize; i++){
        //Uint32 color = rand();
        Uint32 color = i*10000 +10000;

        punto2D_t p1 = poligonosADibujar[i].p1;
        punto2D_t p2 = poligonosADibujar[i].p2;
        punto2D_t p3 = poligonosADibujar[i].p3;
        float z1 = poligonosClipeados[i].p1.z;
        float z2 = poligonosClipeados[i].p2.z;
        float z3 = poligonosClipeados[i].p3.z;
       
        
        int boundingBoxminX = (p1.x < p2.x) ? ((p1.x < p3.x) ? p1.x : p3.x) : ((p2.x < p3.x) ? p2.x : p3.x);
        int boundingBoxminY = (p1.y < p2.y) ? ((p1.y < p3.y) ? p1.y : p3.y) : ((p2.y < p3.y) ? p2.y : p3.y);
        int boundingBoxmaxX = (p1.x > p2.x) ? ((p1.x > p3.x) ? p1.x : p3.x) : ((p2.x > p3.x) ? p2.x : p3.x);
        int boundingBoxmaxY = (p1.y > p2.y) ? ((p1.y > p3.y) ? p1.y : p3.y) : ((p2.y > p3.y) ? p2.y : p3.y);
        
        // este es el clip 2D, que funciona para sacarse los planos de arriba, abajo , izq y der, pero en casos donde haya poligonos muy grandes puede llegar a haber overflow, haciendo que quede raro, por lo que en un futuro habria que hacer todo en 3Dclip
        if (boundingBoxminX < 0) boundingBoxminX = 0;
        if (boundingBoxminY < 0) boundingBoxminY = 0;
        if (boundingBoxmaxX >= ANCHO)  boundingBoxmaxX = ANCHO -1;
        if (boundingBoxmaxY >= ALTURA) boundingBoxmaxY = ALTURA -1;

        int dp1p2y = p1.y - p2.y;
        int dp1p2x = p1.x - p2.x;
        int dp2p3y = p2.y - p3.y;
        int dp2p3x = p2.x - p3.x;
        int dp3y1 = p3.y - p1.y;
        int dp3x1 = p3.x - p1.x;        
        int DobleAreaP1P2P3 = (p1.x - p3.x) * dp2p3y - (p1.y - p3.y) * dp2p3x;
        
        //para la mejor obtimizacion habria que poner este if arriba de todo        
        if (DobleAreaP1P2P3 <= 0) continue; //backfaceculling, esto es igual al componente z de la normal del triangulo, si es negativo el triangulo se aleja de la camara por lo que no se va a notar y se poda

        for(int y = boundingBoxminY; y<= boundingBoxmaxY; y ++ ){ // y +=PIXELSIZE
            for(int x = boundingBoxminX; x<= boundingBoxmaxX; x ++){ // x +=PIXELSIZE
                
                int dobleAreaP1P2P4 = (x - p2.x) * dp1p2y - (y - p2.y) * dp1p2x;
                int dobleAreaP2P3P4 = (x - p3.x) * dp2p3y - (y - p3.y) * dp2p3x;
                int dobleAreaP1P3P4 = (x - p1.x) * dp3y1 - (y - p1.y) * dp3x1;

                float lambda1 = (float) dobleAreaP2P3P4 / DobleAreaP1P2P3;
                float lambda2 = (float) dobleAreaP1P3P4 / DobleAreaP1P2P3;
                float lambda3 = 1.0f - lambda1 - lambda2;
                
                float z = 1.0f/((lambda1*(1.0f/z1)) + (lambda2*(1.0f/z2)) + (lambda3*(1.0f/z3)));
                
                bool positivos = (lambda1 >= 0) && (lambda2 >= 0) && (lambda3 >= 0);
                if(positivos&&(z<=zbuffer[y * ancho_real_memoria + x])){
                    //SDL_Rect pixel = (SDL_Rect){x, y, PIXELSIZE, PIXELSIZE};
                    //SDL_FillRect(surface, &pixel, COLOR_WHITE);
                    zbuffer[y * ancho_real_memoria + x] = z;
                    pixels[y * ancho_real_memoria + x] = color;
                }
                
            }
        }
    }
    SDL_UnlockSurface(surface);
}

punto3D_t interseccionNearPlane(punto3D_t* dentro, punto3D_t* afuera){
        punto3D_t vectorDirecotr = {afuera->x-dentro->x,afuera->y-dentro->y,afuera->z-dentro->z};
        float Intersecccion = (NEARPLANE- afuera->z )/vectorDirecotr.z;
        return (punto3D_t) {vectorDirecotr.x*Intersecccion +afuera->x,vectorDirecotr.y*Intersecccion +afuera->y,NEARPLANE};
}

bool checkeoFueraPantallaPoligono3D(punto3D_t* p1, punto3D_t* p2, punto3D_t* p3){
  

    float z1_margin = p1->z * 3;float z2_margin = p2->z * 3;float z3_margin = p3->z * 3;

    bool p1Arriba = p1->y > z1_margin;  bool p2Arriba = p2->y > z2_margin;  bool p3Arriba = p3->y > z3_margin;
    bool estaArriba = p1Arriba && p2Arriba && p3Arriba;

    bool p1Abajo = p1->y < -z1_margin; bool p2Abajo = p2->y < -z2_margin; bool p3Abajo = p3->y < -z3_margin;
    bool estaAbajo = p1Abajo && p2Abajo && p3Abajo;

    bool p1Derecha = p1->x > z1_margin; bool p2Derecha = p2->x > z2_margin; bool p3Derecha = p3->x > z3_margin;
    bool estaDerecha = p1Derecha && p2Derecha && p3Derecha;

    bool p1Izquierda = p1->x < -z1_margin; bool p2Izquierda = p2->x < -z2_margin; bool p3Izquierda = p3->x < -z3_margin; 
    bool estaIzquierda = p1Izquierda && p2Izquierda && p3Izquierda;

    return estaArriba || estaAbajo || estaDerecha || estaIzquierda;    
}


void caso2Afuera(punto3D_t *p1,punto3D_t *p2,punto3D_t *p3){//asumo p1 p2 afuera
    *p1 = interseccionNearPlane(p3,p1);
    *p2 = interseccionNearPlane(p3,p2);

}
void caso1Afuera(punto3D_t *p1,punto3D_t *p2,punto3D_t *p3, punto3D_t *temp){//asumo p1 p2 afuera
    *temp = interseccionNearPlane(p3,p1);
    *p1 = interseccionNearPlane(p2,p1);
}

void clip3D(modelo_t* modelo){
    poligonosClipeadosSize =0;
    int yay;
    punto3D_t temp = {0};
    for (int i = 0; i < (modelo->cantidadPoligonos); i++){
        
            punto3D_t p1 = VerticesTransformados[modelo->poligonos[i][0]];
            punto3D_t p2 = VerticesTransformados[modelo->poligonos[i][1]];
            punto3D_t p3 = VerticesTransformados[modelo->poligonos[i][2]];

            yay = 0;
            if(p1.z < NEARPLANE){ yay |= 0b001;}
            if(p2.z < NEARPLANE){ yay |= 0b010;}// esto podria ser un enum 
            if(p3.z < NEARPLANE){ yay |= 0b100;}
            switch (yay){
                case 0b000://ninguno afuera
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&p3)){continue;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,p3};
                continue;
                break;
                case 0b001://p1 afuera
                    caso1Afuera(&p1,&p2,&p3,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {temp,p2,p3};}                    
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&temp)){continue;;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,temp};
                break;
                case 0b010://p2 afuera
                    caso1Afuera(&p2,&p1,&p3,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p2,temp,p3};}
                    if(checkeoFueraPantallaPoligono3D(&temp,&p2,&p1)){continue;;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b100://p3 afuera
                    caso1Afuera(&p3,&p2,&p1,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {temp,p2,p3};}
                    if(checkeoFueraPantallaPoligono3D(&temp,&p2,&p1)){continue;;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,temp};
                break;
                case 0b011://p1 p2 afuera
                    caso2Afuera(&p1,&p2,&p3);
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&p3)){continue;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b101://p1 p3 afuera
                    caso2Afuera(&p1,&p3,&p2);
                    if(checkeoFueraPantallaPoligono3D(&p2,&p1,&p3)){continue;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b110://p2 p3 afuera
                    caso2Afuera(&p3,&p2,&p1);
                    if(checkeoFueraPantallaPoligono3D(&p2,&p1,&p3)){continue;}
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b111://p1 p2 p3 afuera
                    continue;
                break;
            }
    }
}
void renderInit(SDL_Surface* superficie,SDL_Window* ventana){
    modelosSize = 0;
    
    parsearModelos(&modelos,&modelosSize);
    
    surface = superficie;
    window = ventana;
    anchoRealMemoria = surface->pitch / 4;
    
    modeloSeleccionado = NULL;
    size_t maxVerticesSize= 0;
    size_t maxPoligonosSize = 0;
    if(modelosSize != 0){
        modeloSeleccionado = &modelos[0];
        modelos[0].posicion = (punto3D_t) {(float) 0.0f,0.0f,0.0f};

        maxVerticesSize= modelos[0].cantidadVertices;
        maxPoligonosSize = modelos[0].cantidadPoligonos;
        for (int i = 1; i< modelosSize;i++ ){
            if(modelos[i].cantidadVertices > maxVerticesSize)  maxVerticesSize =modelos[i].cantidadVertices;
            if(modelos[i].cantidadPoligonos > maxPoligonosSize)  maxPoligonosSize =modelos[i].cantidadPoligonos;
            modelos[i].posicion = (punto3D_t) {(float) (i*1.5),0.0f, 0.0f};
        }
    }
    VerticesTransformados = malloc(sizeof(punto3D_t) * maxVerticesSize);
    poligonosClipeados = malloc(sizeof(poligono3D_t)* maxPoligonosSize *2);
    poligonosADibujar = malloc(sizeof(poligono2D_t)*maxPoligonosSize*2);
    zbuffer = malloc(sizeof(float)*anchoRealMemoria*ALTURA);
    
}
void renderUpdate(){
        SDL_Rect fondo = (SDL_Rect) {0, 0, ANCHO, ALTURA};

        for(int i = 0; i< ALTURA*anchoRealMemoria; i++){zbuffer[i]= INFINITY;}
        
        SDL_FillRect(surface, &fondo, COLOR_BLACK);
        for(int i= 0; i< modelosSize; i++){
            modelo_t modelo = modelos[i];
            transformarVertices(&modelo);
            clip3D(&modelo);
            proyectar2D();
            dibujarPoligono();
        }
        

        SDL_PumpEvents();
        SDL_UpdateWindowSurface(window);
}
void renderInput(const Uint8* teclado){
        if (teclado[SDL_SCANCODE_W]){dz -= DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_S]){dz += DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_A]){dx += DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_D]){dx -= DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_SPACE]){dy -= DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_LSHIFT]){dy += DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_LEFT]){modeloSeleccionado->rotacion += PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_RIGHT]){modeloSeleccionado->rotacion -= PI*DELTATIEMPO;}
};
void renderDestroy(){
    free(VerticesTransformados);
    free(poligonosClipeados);
    free(poligonosADibujar);
    free(zbuffer);
    for(int i = 0; i<modelosSize; i++){
        free(modelos[i].poligonos);
        free(modelos[i].vertices);
    }
    free(modelos);
}
