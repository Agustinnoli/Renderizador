#include <SDL2/SDL.h>
#include <stdbool.h>
#include "render.h"

int main(void){
    
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window* window = SDL_CreateWindow("Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ANCHO, ALTO, 0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    
    renderInit(surface,window);
    
    bool running = true;
    SDL_Event event;    
    const Uint8* teclado = SDL_GetKeyboardState(NULL);

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
        renderInput(teclado);
        renderUpdate();
        SDL_Delay(16);
    }

    renderDestroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}