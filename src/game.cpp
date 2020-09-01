#define SDL_MAIN_HANDLED
#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include "chip8.h"

int main(int argc, char **args) {
    //Chip8 has a 64x32 pixel board. Scale pixel size by WIN_SCALE
    int WIN_SCALE = 8;
    bool screen[64*32];
    for(int i = 0; i<64*32; i++)
        screen[i] = 0;

    std::ifstream gameROM;
    if(argc > 0) {
        gameROM.open(args[1]);
        if(gameROM.good()) {
            std::cout << "opened rom\n";
        } else {
            std::cout << "failed to open rom\n";
        }
    } else {
        std::cout << "Error opening ROM file.\n";
        return -1;
    }


    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error Initiliing SDL\n");
        return -1;
    }

    Chip8 test;
    test.test();

    SDL_Window* window = SDL_CreateWindow( "CYNDI - Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*WIN_SCALE, 32*WIN_SCALE, SDL_WINDOW_SHOWN );
    SDL_Renderer* gameRenderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(gameRenderer,0,0,0,255);
    SDL_Event event;
    bool quit = false;
    SDL_Rect *pixel = new SDL_Rect;
    pixel->x = 0;
    pixel->y = 0;
    pixel->w = WIN_SCALE;
    pixel->h = WIN_SCALE;

    //main loop
    while(!quit) {
        SDL_SetRenderDrawColor(gameRenderer,0,0,0,255); //black
        SDL_RenderClear(gameRenderer);
        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT)
                quit = true;
        }
        SDL_SetRenderDrawColor(gameRenderer,0,255,0,255); // green
        SDL_RenderFillRect(gameRenderer,pixel);
        SDL_RenderPresent(gameRenderer);
        SDL_Delay(500);
        pixel->x +=WIN_SCALE;
    }
    free(pixel);
    SDL_DestroyRenderer(gameRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}