#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "render.h"
#include "parser.h"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000
#define ANCHO 600
#define ALTURA 600
#define PIXELSIZE 1
#define FPS 60
#define DELTATIEMPO 1.0f/FPS
#define PI 3.14159f
#define NEARPLANE 0.1f
#define SENSIBILIDAD 0.002f //ajustado a radianes
#define VELOCIDAD 3.0f * DELTATIEMPO
#define POTENCIA_LUZ 100.0f
float LUZ_AMBIENTAL = 0.2f;

SDL_Surface* surface;
SDL_Window* window;

camara_t camara;

modelo_t* modelos;
size_t modelosSize;

punto3D_t* VerticesTransformados;

poligono3D_t* poligonosClipeados;
size_t poligonosClipeadosSize;

poligono2D_t* poligonosADibujar;

size_t anchoRealMemoria;
float* zbuffer;


int lucesSize;
luz_t* luces;
punto3D_t* lucesTransformadas;


size_t objetoSeleccionado = 0;
objeto_t** objetos;
size_t objetosSize;

static inline punto3D_t normalizar(punto3D_t v) {
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    return (punto3D_t){v.x/len, v.y/len, v.z/len};
}

static inline void transformarPuntos(punto3D_t* input, punto3D_t* output, size_t size,const punto3D_t* posicion,const punto3D_t* rotacion){
    float senox = sin(rotacion->x), cosenox = cos(rotacion->x);
    float senoy = sin(rotacion->y), cosenoy = cos(rotacion->y);
    float senoz = sin(rotacion->z), cosenoz = cos(rotacion->z);

    for(int i = 0; i < size; i++){
        float x1,x2,y1,y2,z1,z2;
        x1 = input[i].x;y1 = input[i].y;z1 = input[i].z;

        //rotar eje z
        x2 = x1 * cosenoz - y1 * senoz;y2 = x1 * senoz + y1 * cosenoz;z2 = z1;
        //rotar eje x
        x1 = x2;y1 = y2 * cosenox - z2 * senox;z1 = y2 * senox + z2 * cosenox;
        //rotar eje y
        x2 = x1 * cosenoy + z1 * senoy;y2 = y1;z2 = -x1 * senoy + z1 * cosenoy;
        // posicion mundo
        x1 = x2 + posicion->x - camara.posicion.x;
        y1 = y2 + posicion->y - camara.posicion.y;
        z1 = z2 + posicion->z - camara.posicion.z;

        //rotar camara
        output[i].x = x1*camara.derecha.x  + y1*camara.derecha.y  + z1*camara.derecha.z;
        output[i].y = x1*camara.arriba.x   + y1*camara.arriba.y   + z1*camara.arriba.z;
        output[i].z = x1*camara.adelante.x + y1*camara.adelante.y + z1*camara.adelante.z;
    }
}


void transformarVertices(modelo_t* modelo){
    transformarPuntos(modelo->vertices, VerticesTransformados, modelo->cantidadVertices, &modelo->objeto.posicion, &modelo->objeto.rotacion);
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

void transformarLuces(){
    punto3D_t origen = {0, 0, 0};
    punto3D_t rotacionCero = {0, 0, 0};
    for(int i = 0; i < lucesSize; i++){
        transformarPuntos(&origen, &lucesTransformadas[i], 1,&luces[i].objeto.posicion, &rotacionCero);
    }
}

float calcularIluminacionPoligono(int i){
    punto3D_t p1 = poligonosClipeados[i].p1;
    punto3D_t p2 = poligonosClipeados[i].p2;
    punto3D_t p3 = poligonosClipeados[i].p3;

    punto3D_t lado1 = {p2.x-p1.x, p2.y-p1.y, p2.z-p1.z};
    punto3D_t lado2 = {p3.x-p1.x, p3.y-p1.y, p3.z-p1.z};

    punto3D_t normal = normalizar((punto3D_t){lado1.y*lado2.z - lado1.z*lado2.y,lado1.z*lado2.x - lado1.x*lado2.z,lado1.x*lado2.y - lado1.y*lado2.x});

    punto3D_t centroPoligono = {(p1.x+p2.x+p3.x)/3.0f,(p1.y+p2.y+p3.y)/3.0f,(p1.z+p2.z+p3.z)/3.0f};

    float intensidad = LUZ_AMBIENTAL;
    for(int j = 0; j < lucesSize; j++){
        float dx = lucesTransformadas[j].x - centroPoligono.x;float dy = lucesTransformadas[j].y - centroPoligono.y;float dz = lucesTransformadas[j].z - centroPoligono.z;
        float distancia = fmax(sqrtf(dx*dx + dy*dy + dz*dz),1e-6f);
        punto3D_t dirLuz = {dx / distancia,dy / distancia,dz / distancia};
        float productoEscalar = fmaxf(0.0f,normal.x * dirLuz.x +normal.y * dirLuz.y +normal.z * dirLuz.z);
        float atenuacion = luces[j].intensidad / (distancia * distancia);
        intensidad += productoEscalar * atenuacion;
    }
    return fminf(intensidad, 1.5f);
}

void dibujarPoligono(){
    if (SDL_LockSurface(surface) != 0) {return;}
    size_t ancho_real_memoria = surface->pitch / 4;
    Uint32 *pixels = (Uint32 *)surface->pixels;
    srand(0); 
    for (int i = 0; i < poligonosClipeadosSize; i++){
        //Uint32 color = rand();
        

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
        
        //para la mejor optimizacion habria que poner este if arriba de todo        
        if (DobleAreaP1P2P3 <= 0) continue; //backfaceculling, esto es igual al componente z de la normal del triangulo, si es negativo el triangulo se aleja de la camara por lo que no se va a notar y se poda

        float intensidadLuz = calcularIluminacionPoligono(i);

        for(int y = boundingBoxminY; y<= boundingBoxmaxY ; y ++ ){ // y +=PIXELSIZE
            for(int x = boundingBoxminX; x<= boundingBoxmaxX ; x ++){ // x +=PIXELSIZE
                
                int dobleAreaP1P2P4 = (x - p2.x) * dp1p2y - (y - p2.y) * dp1p2x;
                int dobleAreaP2P3P4 = (x - p3.x) * dp2p3y - (y - p3.y) * dp2p3x;
                int dobleAreaP1P3P4 = (x - p1.x) * dp3y1 - (y - p1.y) * dp3x1;

                float lambda1 = (float) dobleAreaP2P3P4 / DobleAreaP1P2P3;
                float lambda2 = (float) dobleAreaP1P3P4 / DobleAreaP1P2P3;
                float lambda3 = 1.0f - lambda1 - lambda2;
                
                float z = 1.0f/((lambda1*(1.0f/z1)) + (lambda2*(1.0f/z2)) + (lambda3*(1.0f/z3)));
                
                Uint8 r = (Uint8)(fminf(155.0f* intensidadLuz, 255.0f));Uint8 g = (Uint8)(fminf(0.0f* intensidadLuz, 255.0f));Uint8 b = (Uint8)(fminf(0.0f * intensidadLuz, 255.0f));
                Uint32 color = (r << 16) | (g << 8) | b;

                bool positivos = (lambda1 >= -0.0001f) && (lambda2 >= -0.0001f) && (lambda3 >= -0.0001f);
                if(positivos&&(z<=zbuffer[y * ancho_real_memoria + x])){
                    zbuffer[y * ancho_real_memoria + x] = z;
                    pixels[y * ancho_real_memoria + x] = color;
                }
                //wireframe
                //if((fabsf(lambda1) <= 0.02f || fabsf(lambda2) <= 0.02f || fabsf(lambda3) <= 0.02f)&& positivos&& (z < zbuffer[y * ancho_real_memoria + x] + 0.01f)){ pixels[y * ancho_real_memoria + x] = COLOR_WHITE;}
                
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
    float z1Margen = p1->z * 3;float z2Margen = p2->z * 3;float z3Margen = p3->z * 3;

    bool p1Arriba = p1->y > z1Margen;  bool p2Arriba = p2->y > z2Margen;  bool p3Arriba = p3->y > z3Margen;
    bool estaArriba = p1Arriba && p2Arriba && p3Arriba;

    bool p1Abajo = p1->y < -z1Margen; bool p2Abajo = p2->y < -z2Margen; bool p3Abajo = p3->y < -z3Margen;
    bool estaAbajo = p1Abajo && p2Abajo && p3Abajo;

    bool p1Derecha = p1->x > z1Margen; bool p2Derecha = p2->x > z2Margen; bool p3Derecha = p3->x > z3Margen;
    bool estaDerecha = p1Derecha && p2Derecha && p3Derecha;

    bool p1Izquierda = p1->x < -z1Margen; bool p2Izquierda = p2->x < -z2Margen; bool p3Izquierda = p3->x < -z3Margen; 
    bool estaIzquierda = p1Izquierda && p2Izquierda && p3Izquierda;

    //return false;
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

            if(checkeoFueraPantallaPoligono3D(&p1,&p2,&p3)){continue;}
            yay = 0;
            if(p1.z < NEARPLANE){ yay |= 0b001;}
            if(p2.z < NEARPLANE){ yay |= 0b010;}// esto podria ser un enum 
            if(p3.z < NEARPLANE){ yay |= 0b100;}
            switch (yay){
                case 0b000://ninguno afuera
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p3,p2};
                continue;
                break;
                case 0b001://p1 afuera
                    caso1Afuera(&p1,&p2,&p3,&temp);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {temp,p3,p2};                   
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,temp,p2};
                break;
                case 0b010://p2 afuera
                    caso1Afuera(&p2,&p1,&p3,&temp);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p2,p3,temp};
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p3,p2};
                break;
                case 0b100://p3 afuera
                    caso1Afuera(&p3,&p2,&p1,&temp);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {temp,p3,p2};
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,temp,p2};
                break;
                case 0b011://p1 p2 afuera
                    caso2Afuera(&p1,&p2,&p3);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p3,p2};
                break;
                case 0b101://p1 p3 afuera
                    caso2Afuera(&p1,&p3,&p2);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p3,p2};
                break;
                case 0b110://p2 p3 afuera
                    caso2Afuera(&p3,&p2,&p1);
                    poligonosClipeados[poligonosClipeadosSize++] = (poligono3D_t) {p1,p3,p2};
                break;
                case 0b111://p1 p2 p3 afuera
                    continue;
                break;
            }
    }
}
void modelosEncontrarBoundingBox(modelo_t* modelos) {
    for (int i = 0; i < modelosSize; i++) {
        modelo_t* modelo = &modelos[i]; 
        
        float minX = modelo->vertices[0].x, maxX = modelo->vertices[0].x;
        float minY = modelo->vertices[0].y, maxY = modelo->vertices[0].y;
        float minZ = modelo->vertices[0].z, maxZ = modelo->vertices[0].z;

        for (int j = 1; j < modelo->cantidadVertices; j++) {
            punto3D_t p = modelo->vertices[j];
            if (p.x < minX) minX = p.x; if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y; if (p.y > maxY) maxY = p.y;
            if (p.z < minZ) minZ = p.z; if (p.z > maxZ) maxZ = p.z;
        }

        (*modelo->boundingBox)[minXminYminZ] = (punto3D_t){minX, minY, minZ};
        (*modelo->boundingBox)[maxXminYminZ] = (punto3D_t){maxX, minY, minZ};
        (*modelo->boundingBox)[minXmaxYminZ] = (punto3D_t){minX, maxY, minZ};
        (*modelo->boundingBox)[minXminYmaxZ] = (punto3D_t){minX, minY, maxZ};
        (*modelo->boundingBox)[maxXmaxYminZ] = (punto3D_t){maxX, maxY, minZ};
        (*modelo->boundingBox)[minXmaxYmaxZ] = (punto3D_t){minX, maxY, maxZ};
        (*modelo->boundingBox)[maxXminYmaxZ] = (punto3D_t){maxX, minY, maxZ};
        (*modelo->boundingBox)[maxXmaxYmaxZ] = (punto3D_t){maxX, maxY, maxZ};
    }
}

void renderInit(SDL_Surface* superficie,SDL_Window* ventana, int cantLuces){
    modelosSize = 0;
    parsearModelos(&modelos,&modelosSize);
    modelosEncontrarBoundingBox(modelos);

    surface = superficie;
    window = ventana;
    
    camara = (camara_t) {{0.0f, 0.0f, 2.0f}, {0,0,-1}, {0,1,0}, {1,0,0}};

    anchoRealMemoria = surface->pitch / 4;
    
    size_t maxVerticesSize= 0;
    size_t maxPoligonosSize = 0;
    if(modelosSize != 0){
        modelos[0].objeto.posicion = (punto3D_t) {(float) 0.0f,0.0f,0.0f};

        maxVerticesSize= modelos[0].cantidadVertices;
        maxPoligonosSize = modelos[0].cantidadPoligonos;
        for (int i = 1; i< modelosSize;i++ ){
            if(modelos[i].cantidadVertices > maxVerticesSize)  maxVerticesSize =modelos[i].cantidadVertices;
            if(modelos[i].cantidadPoligonos > maxPoligonosSize)  maxPoligonosSize =modelos[i].cantidadPoligonos;
            modelos[i].objeto.posicion = (punto3D_t) {(float) (i*1.5),(i*1.5), (i*1.5)};
        }

    }
    VerticesTransformados = malloc(sizeof(punto3D_t) * maxVerticesSize);
    poligonosClipeados = malloc(sizeof(poligono3D_t)* maxPoligonosSize *2);
    poligonosADibujar = malloc(sizeof(poligono2D_t)*maxPoligonosSize*2);
    zbuffer = malloc(sizeof(float)*anchoRealMemoria*ALTURA);
    
    lucesSize = cantLuces; if(lucesSize==0){LUZ_AMBIENTAL = 1.0f;}
    luces= calloc(sizeof(luz_t),cantLuces);
    for(int i = 0; i<lucesSize; i++){luces[i].intensidad = POTENCIA_LUZ;luces[i].objeto.posicion = (punto3D_t) {6.0f, 6.0f, 6.0f};}
    lucesTransformadas = malloc(sizeof(luz_t)*cantLuces);
    
    objetos = malloc(sizeof(objeto_t*)*(modelosSize+lucesSize));
    for(int i = 0; i<modelosSize; i++){objetos[i] = &modelos[i].objeto;}
    for(int i = 0; i<lucesSize; i++){objetos[modelosSize+i] = &luces[i].objeto;}
    objetosSize =modelosSize+lucesSize;
    
}

bool modeloFueraDelFrustrum(modelo_t* modelo) {
    punto3D_t transformados[BoundingBoxSize];
    transformarPuntos(*modelo->boundingBox, transformados,BoundingBoxSize, &modelo->objeto.posicion, &modelo->objeto.rotacion);

    int fueraIzquierda=0, fueraDerecha=0, fueraArriba=0, fueraAbajo=0, fueraNear=0;
    for (int i = 0; i < BoundingBoxSize; i++) {
        punto3D_t p = transformados[i];
        if (p.z < NEARPLANE)  fueraNear++;
        if (p.x < -p.z) fueraIzquierda++;
        else if (p.x > p.z) fueraDerecha++;
        if (p.y < -p.z) fueraAbajo++;
        else if (p.y > p.z) fueraArriba++;
    }

    return (fueraNear    == BoundingBoxSize) || (fueraIzquierda == BoundingBoxSize) || (fueraDerecha   == BoundingBoxSize) || (fueraAbajo     == BoundingBoxSize) || (fueraArriba    == BoundingBoxSize);

}


void renderUpdate(){
        SDL_Rect fondo = (SDL_Rect) {0, 0, ANCHO, ALTURA};
        
        for(int i = 0; i< ALTURA*anchoRealMemoria; i++){zbuffer[i]= INFINITY;}
        
        SDL_FillRect(surface, &fondo, COLOR_BLACK);
        
        transformarLuces();
        
        for(int i= 0; i< modelosSize; i++){
            modelo_t* modelo = &modelos[i];
            if(modeloFueraDelFrustrum(modelo)) continue;
            transformarVertices(modelo);
            clip3D(modelo);
            proyectar2D();
            dibujarPoligono();
        }
        SDL_PumpEvents();
        SDL_UpdateWindowSurface(window);
}


static punto3D_t rotar(const punto3D_t* v, const punto3D_t* eje, float angulo) {
    float coseno = cos(angulo), seno = sin(angulo);
    float productoCruz = eje->x*v->x + eje->y*v->y + eje->z*v->z;
    punto3D_t cruz = {
        eje->y*v->z - eje->z*v->y,
        eje->z*v->x - eje->x*v->z,
        eje->x*v->y - eje->y*v->x
    };
    return (punto3D_t){
        v->x*coseno + cruz.x*seno + eje->x*productoCruz*(1-coseno),
        v->y*coseno + cruz.y*seno + eje->y*productoCruz*(1-coseno),
        v->z*coseno + cruz.z*seno + eje->z*productoCruz*(1-coseno)
    };
}



void actualizarCamara(float deltaX, float deltaY) {

    punto3D_t ejeY = {0, 1, 0};
    camara.adelante = normalizar(rotar(&camara.adelante, &ejeY, -deltaX * SENSIBILIDAD));
    camara.derecha  = normalizar(rotar(&camara.derecha,  &ejeY, -deltaX * SENSIBILIDAD));
    camara.adelante = normalizar(rotar(&camara.adelante, &camara.derecha, deltaY * SENSIBILIDAD));

    camara.arriba = (punto3D_t){
        camara.derecha.y * camara.adelante.z - camara.derecha.z * camara.adelante.y,
        camara.derecha.z * camara.adelante.x - camara.derecha.x * camara.adelante.z,
        camara.derecha.x * camara.adelante.y - camara.derecha.y * camara.adelante.x
    };
}
void renderInput(const Uint8* teclado){
        if (teclado[SDL_SCANCODE_W]) {
            camara.posicion.x += camara.adelante.x * VELOCIDAD;
            camara.posicion.y += camara.adelante.y * VELOCIDAD; 
            camara.posicion.z += camara.adelante.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_S]) {
            camara.posicion.x -= camara.adelante.x * VELOCIDAD;
            camara.posicion.y -= camara.adelante.y * VELOCIDAD;
            camara.posicion.z -= camara.adelante.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_A]) {
            camara.posicion.x -= camara.derecha.x * VELOCIDAD;
            camara.posicion.z -= camara.derecha.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_D]) {
            camara.posicion.x += camara.derecha.x * VELOCIDAD;
            camara.posicion.z += camara.derecha.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_SPACE]) {
            camara.posicion.x += camara.arriba.x * VELOCIDAD;
            camara.posicion.y += camara.arriba.y * VELOCIDAD;
            camara.posicion.z += camara.arriba.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_LSHIFT]) {
            camara.posicion.x -= camara.arriba.x * VELOCIDAD;
            camara.posicion.y -= camara.arriba.y * VELOCIDAD;
            camara.posicion.z -= camara.arriba.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_R]) {
            objetos[objetoSeleccionado]->posicion.x += camara.adelante.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.y += camara.adelante.y * VELOCIDAD; 
            objetos[objetoSeleccionado]->posicion.z += camara.adelante.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_Y]) {
            objetos[objetoSeleccionado]->posicion.x -= camara.adelante.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.y -= camara.adelante.y * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.z -= camara.adelante.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_F]) {
            objetos[objetoSeleccionado]->posicion.x -= camara.derecha.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.z -= camara.derecha.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_H]) {
            objetos[objetoSeleccionado]->posicion.x += camara.derecha.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.z += camara.derecha.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_T]) {
            objetos[objetoSeleccionado]->posicion.x += camara.arriba.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.y += camara.arriba.y * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.z += camara.arriba.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_G]) {
            objetos[objetoSeleccionado]->posicion.x -= camara.arriba.x * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.y -= camara.arriba.y * VELOCIDAD;
            objetos[objetoSeleccionado]->posicion.z -= camara.arriba.z * VELOCIDAD;
        }
        if (teclado[SDL_SCANCODE_E]){objetoSeleccionado = (objetoSeleccionado == 0) ?objetosSize - 1 :objetoSeleccionado - 1;}
        if (teclado[SDL_SCANCODE_Q]){objetoSeleccionado = (objetoSeleccionado + 1)% objetosSize;}
        if (teclado[SDL_SCANCODE_I]){objetos[objetoSeleccionado]->rotacion.x += PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_K]){objetos[objetoSeleccionado]->rotacion.x -= PI*DELTATIEMPO;} 
        if (teclado[SDL_SCANCODE_J]){objetos[objetoSeleccionado]->rotacion.y += PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_L]){objetos[objetoSeleccionado]->rotacion.y -= PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_U]){objetos[objetoSeleccionado]->rotacion.z += PI*DELTATIEMPO;}
        if (teclado[SDL_SCANCODE_O]){objetos[objetoSeleccionado]->rotacion.z -= PI*DELTATIEMPO;}
};
void renderDestroy(){
    free(VerticesTransformados);
    free(poligonosClipeados);
    free(poligonosADibujar);
    free(zbuffer);
    free(luces);
    free(lucesTransformadas);
    for(int i = 0; i<modelosSize; i++){
        free(modelos[i].poligonos);
        free(modelos[i].vertices);
        free(modelos[i].boundingBox);
    }
    free(modelos);
}
