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
float areaPoligono2D(){

}

void dibujarPoligono(){
    if (SDL_LockSurface(surface) != 0) {return;}
    size_t ancho_real_memoria = surface->pitch / 4;
    Uint32 *pixels = (Uint32 *)surface->pixels;
    srand(0); 
    for (int i = 0; i < tamanioPoligonosClipeados; i++){
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


void caso2Afuera(punto3D_t *p1,punto3D_t *p2,punto3D_t *p3){//asumo p1 p2 afuera
    *p1 = jijona(p3,p1);
    *p2 = jijona(p3,p2);

}
void caso1Afuera(punto3D_t *p1,punto3D_t *p2,punto3D_t *p3, punto3D_t *temp){//asumo p1 p2 afuera
    *temp = jijona(p3,p1);
    *p1 = jijona(p2,p1);
}

void clip3D(){
    tamanioPoligonosClipeados =0;
    int yay;
    punto3D_t temp = {0};
    for (int i = 0; i < (sizeof(poligonos) / sizeof(int[3])); i++){

            punto3D_t p1 = poligonosRotados[i].p1;
            punto3D_t p2 = poligonosRotados[i].p2;
            punto3D_t p3 = poligonosRotados[i].p3;
            yay = 0;
            if(p1.z < NEARPLANE){ yay |= 0b001;}
            if(p2.z < NEARPLANE){ yay |= 0b010;}// esto podria ser un enum 
            if(p3.z < NEARPLANE){ yay |= 0b100;}
            switch (yay){
                case 0b000://ninguno afuera
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&p3)){continue;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,p3};
                continue;
                break;
                case 0b001://p1 afuera
                    caso1Afuera(&p1,&p2,&p3,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {temp,p2,p3};}                    
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&temp)){continue;;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,temp};
                break;
                case 0b010://p2 afuera
                    caso1Afuera(&p2,&p1,&p3,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p2,temp,p3};}
                    if(checkeoFueraPantallaPoligono3D(&temp,&p2,&p1)){continue;;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b100://p3 afuera
                    caso1Afuera(&p3,&p2,&p1,&temp);
                    if(!checkeoFueraPantallaPoligono3D(&temp,&p2,&p3)){poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {temp,p2,p3};}
                    if(checkeoFueraPantallaPoligono3D(&temp,&p2,&p1)){continue;;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,temp};
                break;
                case 0b011://p1 p2 afuera
                    caso2Afuera(&p1,&p2,&p3);
                    if(checkeoFueraPantallaPoligono3D(&p1,&p2,&p3)){continue;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b101://p1 p3 afuera
                    caso2Afuera(&p1,&p3,&p2);
                    if(checkeoFueraPantallaPoligono3D(&p2,&p1,&p3)){continue;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b110://p2 p3 afuera
                    caso2Afuera(&p3,&p2,&p1);
                    if(checkeoFueraPantallaPoligono3D(&p2,&p1,&p3)){continue;}
                    poligonosClipeados[tamanioPoligonosClipeados++] = (poligono3D_t) {p1,p2,p3};
                break;
                case 0b111://p1 p2 p3 afuera
                    continue;
                break;
            }
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

