#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>

int main() {

    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error Initiliing SDL\n");
        return -1;
    }

    SDL_Window * window = NULL;
    window = SDL_CreateWindow( "CYNDI - Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*8, 32*8, SDL_WINDOW_SHOWN );
    SDL_Event event;
    bool quit = false;

    while(!quit) {
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT)
                quit = true;
        }
    }
    return 0;
}