#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>

int main() {

	if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error Initiliing SDL\n");
        return -1;
    }

    SDL_Window * window = NULL;
    window = SDL_CreateWindow( "Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*4, 32*4, SDL_WINDOW_SHOWN );
    int x;
    std::cin >> x;
	return 0;
}