#define SDL_MAIN_HANDLED
#include <iostream>
#include <cstdint>
#include <SDL2/SDL.h>
#include "chip8.h"

void printBoard(bool *board); // prints an ASCII board to console for debugging

int main(int argc, char **args) {
  if(argc == 1) {
    std::cout << "Enter ROM title when executing\n";
    return -1;
  }

  //Chip8 has a 64x32 pixel board. Scale pixel size by WIN_SCALE
  int WIN_SCALE = 8;
  bool screen[PIX_COUNT]; //1 pixel is 1 bit
  for(int i = 0; i < PIX_COUNT; i++)
    screen[i] = false;

  if(SDL_Init(SDL_INIT_VIDEO)) {
    printf("Error Initiliing SDL\n");
    return -1;
  }

  Chip8 cpu(screen);
  if(cpu.loadROM(args[1])) {
    std::cout << "Error opening ROM\n";
    return -1;
  }

  //set up game window and pixel
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
    int t = 0;
    SDL_SetRenderDrawColor(gameRenderer,0,0,0,255); //black
    SDL_RenderClear(gameRenderer);
    while(SDL_PollEvent(&event) != 0) {
      if(event.type == SDL_QUIT)
        quit = true;
    }

    SDL_SetRenderDrawColor(gameRenderer,0,255,0,255); // green
    for(t = 0; t < PIX_COUNT; t++) {
      if (screen[t]) {
        pixel->x = (t % PIX_WIDTH) * WIN_SCALE;
        pixel->y = (t / PIX_WIDTH) * WIN_SCALE;
        SDL_RenderFillRect(gameRenderer,pixel);
      }
    }

    SDL_RenderPresent(gameRenderer);
    SDL_Delay(500);
  }
  free(pixel);
  SDL_DestroyRenderer(gameRenderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

void printBoard(bool *board) {
  std::cout << "Start board\n";
  for(int j = 0; j < PIX_HEIGHT; j++) {
    for(int i = 0; i < PIX_WIDTH; i++) {
      if(board[j*PIX_WIDTH + i]) {
        std::cout << "X";
      } else {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << "End board\n";
  return;
}