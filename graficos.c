#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

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

poligono3D_t* poligonosRotados;
poligono3D_t* poligonosClipeados;
poligono2D_t* poligonosADibujar;

int tamanioPoligonosClipeados;

punto3D_t vertices[] = {
    {-0.5f, -0.5f, -0.5f}, 
    { 0.5f, -0.5f, -0.5f}, 
    { 0.5f,  0.5f, -0.5f}, 
    {-0.5f,  0.5f, -0.5f}, 
    {-0.5f, -0.5f,  0.5f}, 
    { 0.5f, -0.5f,  0.5f}, 
    { 0.5f,  0.5f,  0.5f}, 
    {-0.5f,  0.5f,  0.5f}  
};

int poligonos[][3] = {
    {0, 3, 2},
    {0, 2, 1},
    {4, 5, 6},
    {4, 6, 7},
    {4, 7, 3},
    {4, 3, 0},
    {1, 2, 6},
    {1, 6, 5},
    {3, 7, 6},
    {3, 6, 2},
    {4, 0, 1},
    {4, 1, 5}
};

float* zbuffer;


void ajustar(){
    for(int i =0 ; i < sizeof(poligonos) / sizeof(poligonos[0]); i ++ ){
        punto3D_t* puntos = (punto3D_t*)&poligonosRotados[i];
        for (int j = 0; j < 3; j++) {
            puntos[j].x += dx;
            puntos[j].y += dy;
            puntos[j].z += dz;
        }
    }
}
void generarRotacion(){
    float seno = sin(angulo);
    float coseno = cos(angulo);
    for(int i =0 ; i < sizeof(poligonos) / sizeof(poligonos[0]); i ++ ){
        punto3D_t* puntos = (punto3D_t*)&poligonosRotados[i];
        for (int j = 0; j < 3; j++) {
            puntos[j].x = vertices[poligonos[i][j]].x * coseno - vertices[poligonos[i][j]].z * seno;
            puntos[j].y = vertices[poligonos[i][j]].y;
            puntos[j].z = vertices[poligonos[i][j]].x * seno + vertices[poligonos[i][j]].z * coseno;
        }
    }
        //poligonosRotados[i].x =vertices[i].x * cos(angulo) - vertices[i].y * sin(angulo);
        //poligonosRotados[i].y =vertices[i].x * sin(angulo) + vertices[i].y * cos(angulo);
        //poligonosRotados[i].z =vertices[i].z;
        //poligonosRotados[i].x =vertices[i].x;
        //poligonosRotados[i].y =vertices[i].y * cos(angulo) - vertices[i].z * sin(angulo);
        //poligonosRotados[i].z =vertices[i].y * sin(angulo) + vertices[i].z * cos(angulo);
}
punto2D_t coordsAPantalla(float x, float y){
    return (punto2D_t) {(int)((x + 1) * ANCHO - PIXELSIZE)/2,(int) ((1 - y) * ALTURA - PIXELSIZE)/2} ;  
}
void proyectarAPantalla(){

    for(int i =0 ; i < tamanioPoligonosClipeados; i ++ ){
        punto2D_t* pts_dibujar = (punto2D_t*) &poligonosADibujar[i].p1;
        punto3D_t* pts_clipeados = (punto3D_t*)&poligonosClipeados[i];
        for (int j = 0; j < 3; j++) {
            float x =  pts_clipeados[j].x / pts_clipeados[j].z;
            float y =  pts_clipeados[j].y / pts_clipeados[j].z;
            pts_dibujar[j] = coordsAPantalla(x,y);
        }
    }
}
void dibujarPoligono(){
    if (SDL_LockSurface(surface) != 0) {return;}
    size_t ancho_real_memoria = surface->pitch / 4;
    Uint32 *pixels = (Uint32 *)surface->pixels;
    srand(0); 
    for (int i = 0; i < tamanioPoligonosClipeados; i++){
        //Uint32 color = rand();
        Uint32 color = i*10000 +10000;

        punto2D_t *p1 = &poligonosADibujar[i].p1;
        punto2D_t *p2 = &poligonosADibujar[i].p2;
        punto2D_t *p3 = &poligonosADibujar[i].p3;
        float z1 = poligonosClipeados[i].p1.z;
        float z2 = poligonosClipeados[i].p2.z;
        float z3 = poligonosClipeados[i].p3.z;
       
        int x1 = (int) p1->x;
        int y1 = (int) p1->y;
        int x2 = (int) p2->x;
        int y2 = (int) p2->y;
        int x3 = (int) p3->x;
        int y3 = (int) p3->y;
        float z = (z1 + z2 + z3) / 3.0f;
        
        int boundingBoxminX = (x1 < x2) ? ((x1 < x3) ? x1 : x3) : ((x2 < x3) ? x2 : x3);
        int boundingBoxminY = (y1 < y2) ? ((y1 < y3) ? y1 : y3) : ((y2 < y3) ? y2 : y3);
        int boundingBoxmaxX = (x1 > x2) ? ((x1 > x3) ? x1 : x3) : ((x2 > x3) ? x2 : x3);
        int boundingBoxmaxY = (y1 > y2) ? ((y1 > y3) ? y1 : y3) : ((y2 > y3) ? y2 : y3);
        
        // este es el clip 2D, que funciona para sacarse los planos de arriba, abajo , izq y der, pero en casos donde haya poligonos muy grandes puede llegar a haber overflow, haciendo que quede raro, por lo que en un futuro habria que hacer todo en 3Dclip
        if (boundingBoxminX < 0) boundingBoxminX = 0;
        if (boundingBoxminY < 0) boundingBoxminY = 0;
        if (boundingBoxmaxX >= ANCHO)  boundingBoxmaxX = ANCHO -1;
        if (boundingBoxmaxY >= ALTURA) boundingBoxmaxY = ALTURA -1;

        int d1,d2,d3;
        int dy12 = y1 - y2;
        int dx12 = x1 - x2;
        int dy23 = y2 - y3;
        int dx23 = x2 - x3;
        int dy31 = y3 - y1;
        int dx31 = x3 - x1;        

        for(int y = boundingBoxminY; y<= boundingBoxmaxY; y ++ ){ // y +=PIXELSIZE
            for(int x = boundingBoxminX; x<= boundingBoxmaxX; x ++){ // x +=PIXELSIZE
                d1 = (x - x2) * dy12 - (y - y2) * dx12;
                d2 = (x - x3) * dy23 - (y - y3) * dx23;
                if ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) {continue;}
                d3 = (x - x1) * dy31 - (y - y1) * dx31;
                int tiene_negativos = (d1 < 0) || (d2 < 0) || (d3 < 0);
                int tiene_positivos = (d1 > 0) || (d2 > 0) || (d3 > 0);
                if((!(tiene_negativos&&tiene_positivos))&&(z<zbuffer[y * ancho_real_memoria + x])){
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

punto3D_t jijona(punto3D_t* dentro, punto3D_t* afuera){
        punto3D_t vectorDirecotr = {afuera->x-dentro->x,afuera->y-dentro->y,afuera->z-dentro->z};
        float Intersecccion = (NEARPLANE- afuera->z )/vectorDirecotr.z;
        return (punto3D_t) {vectorDirecotr.x*Intersecccion +afuera->x,vectorDirecotr.y*Intersecccion +afuera->y,NEARPLANE};
}

bool checkeoFueraPantallaPoligono3D(punto3D_t* p1,punto3D_t* p2,punto3D_t* p3){
    bool p1Arriba = p1->y > p1->z;bool p2Arriba = p2->y > p2->z;bool p3Arriba = p3->y > p3->z;
    bool estaArriba = p1Arriba&&p2Arriba&&p3Arriba;

    bool p1Abajo = p1->y < -p1->z;bool p2Abajo = p2->y < -p2->z;bool p3Abajo = p3->y < -p3->z;
    bool estaAbajo = p1Abajo&&p2Abajo&&p3Abajo;

    bool p1Derecha = p1->x > p1->z;bool p2Derecha = p2->x > p2->z;bool p3Derecha = p3->x >  p3->z;
    bool estaDerecha =p1Derecha&&p2Derecha&&p3Derecha;

    bool p1Izquierda = p1->x < -p1->z;bool p2Izquierda = p2->x < -p2->z;bool p3Izquierda = p3->x < -p3->z; 
    bool estaIzqiuerda = p1Izquierda&&p2Izquierda&&p3Izquierda;

    return estaArriba||estaAbajo||estaDerecha||estaIzqiuerda;    
}
void clip3D(){
    tamanioPoligonosClipeados =0;
    int yay;
    punto3D_t* temp;
    punto3D_t temp2;
    punto3D_t temp3;
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){

            punto3D_t *p1 = &poligonosRotados[i].p1;
            punto3D_t *p2 = &poligonosRotados[i].p2;
            punto3D_t *p3 = &poligonosRotados[i].p3;
            yay = 0;
            if(p1->z < NEARPLANE){ yay |= 0b001;}
            if(p2->z < NEARPLANE){ yay |= 0b010;}// esto podria ser un enum 
            if(p3->z < NEARPLANE){ yay |= 0b100;}
            switch (yay){
                case 0b000://ninguno afuera
                    if(checkeoFueraPantallaPoligono3D(p1,p2,p3)){continue;}
                    poligonosClipeados[tamanioPoligonosClipeados] = (poligono3D_t) {*p1,*p2,*p3};
                    tamanioPoligonosClipeados++;
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
                    poligonosClipeados[tamanioPoligonosClipeados] = (poligono3D_t) {temp2,*p2,*p3};
                    tamanioPoligonosClipeados++;
                }
                temp3 = jijona(p2,p1);
                if(checkeoFueraPantallaPoligono3D(&temp3,p2,&temp2)){continue;}
                poligonosClipeados[tamanioPoligonosClipeados] = (poligono3D_t) {temp3,*p2,temp2};
                tamanioPoligonosClipeados++;
                continue; 

            caso2Afuera: // asumo p1 p2 afuera;
                temp2 = jijona(p3,p1);
                temp3 = jijona(p3,p2);
                if(checkeoFueraPantallaPoligono3D(&temp3,&temp2,p3)){continue;}
                poligonosClipeados[tamanioPoligonosClipeados] = (poligono3D_t) {temp3,temp2,*p3};
                tamanioPoligonosClipeados++;
                continue;
        
    }
    
}



int main(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,ALTURA ,ANCHO, 0);
    surface = SDL_GetWindowSurface(window);
    SDL_Rect fondo = (SDL_Rect) {0, 0, ANCHO, ALTURA};
    bool running = true;
    SDL_Event event;
    const Uint8 *teclado = SDL_GetKeyboardState(NULL);
    poligonosClipeados = malloc((sizeof(poligono3D_t)*sizeof(poligonos)*2)/sizeof(int[3]));
    poligonosADibujar = malloc((sizeof(poligono2D_t)*sizeof(poligonos)*2)/sizeof(int[3]));
    size_t ancho_real_memoria = surface->pitch / 4;
    zbuffer = malloc(sizeof(float)*ancho_real_memoria*ALTURA);
    
    poligonosRotados = malloc(sizeof(poligono3D_t) * (sizeof(poligonos) / sizeof(poligonos[0])));
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

        for(int i = 0; i< ALTURA*ancho_real_memoria; i++){zbuffer[i]= INFINITY;}

        generarRotacion();
        ajustar();
        clip3D();
        proyectarAPantalla();
        
        dibujarPoligono();

        SDL_PumpEvents();
        SDL_UpdateWindowSurface(window);
        SDL_Delay(16); //60FPS
        SDL_FillRect(surface, &fondo, COLOR_BLACK); 
    }
    free(poligonosRotados);
    free(poligonosClipeados);
    free(poligonosADibujar);
    free(zbuffer);
    return 0;
}

