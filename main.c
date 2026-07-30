#include <SDL2/SDL.h>
#include <stdbool.h>
#include "render.h"
#include "parser.h"

void actualizarContadorFrames(Uint32 *tiempoAnterior, int *frames, float *acumulador){
    Uint32 tiempoActual = SDL_GetTicks();
    float delta = (tiempoActual - *tiempoAnterior) / 1000.0f;
    *tiempoAnterior = tiempoActual;
    *acumulador += delta;
    (*frames)++;
    if(*acumulador >= 1.0f){
        printf("FPS: %d\n", *frames);
        fflush(stdout);
        *frames = 0;
        *acumulador = 0.0f;
    }
}

int main(int argc, char *argv[]){
    int cantLuces;
    if (argc > 2) {printf("usar solo 1 argumento\n");return 1;}
    else if  (argc == 2){cantLuces  = atoi(argv[1]);}
    else {printf("añadir argumento de cuantas luces usar\n");return 1;}
    

    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ANCHO, ALTO, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    SDL_RaiseWindow(window);

    renderInit(surface,window,cantLuces);
    
    bool running = true;
    SDL_Event event;    
    const Uint8* teclado = SDL_GetKeyboardState(NULL);
    
    Uint32 tiempoAnterior = SDL_GetTicks();
    int frames = 0;
    float acumulador = 0;


    while(running){
        actualizarContadorFrames(&tiempoAnterior,&frames,&acumulador);

        while(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){running = false;}
            else if ((event.type == SDL_MOUSEMOTION)&&(event.motion.state & SDL_BUTTON_LMASK) ){actualizarCamara(event.motion.xrel, event.motion.yrel);}            
        }
        renderInput(teclado);
        renderUpdate();
    }

    renderDestroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}