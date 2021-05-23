#define SDL_MAIN_HANDLED
#include <iostream>
#include <cstdint>
#include <SDL2/SDL.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include "chip8.h"

bool DEBUG_MODE = false;
bool FIND_MODE = false;

void printBoard(int *board); // prints an ASCII board to console for debugging

int main(int argc, char **args) {
  std::srand(std::time(NULL));
  if(argc == 1) {
    std::cout << "Enter ROM title when executing\n";
    return -1;
  }

  //debug mode
  if(argc > 2) {
    for(int i = 2; i < argc; i++) {
      if (strcmp(args[i],"debug") == 0)
        DEBUG_MODE = true;
      if(strcmp(args[i],"find") == 0)
        FIND_MODE = true;
    }
  }

  //Chip8 has a 64x32 pixel board. Scale pixel size by WIN_SCALE
  int WIN_SCALE = 8;
  int screen[PIX_COUNT]; //1 pixel is 1 bit
  for(int i = 0; i < PIX_COUNT; i++)
    screen[i] = 0;

  if(SDL_Init(SDL_INIT_VIDEO)) {
    printf("Error Initiliing SDL\n");
    return -1;
  }

  Chip8 cpu;
  if(cpu.loadROM(args[1])) {
    std::cout << "Error opening ROM\n";
    return -1;
  }

  int backgroundRGB[3];
  if(cpu.areCustomColors()) {
    cpu.getBackgroundRGB(backgroundRGB);
  } else {
    backgroundRGB[0] = 0;
    backgroundRGB[1] = 0;
    backgroundRGB[2] = 0;
  }

  //set up game window and pixel
  SDL_Window* window = SDL_CreateWindow( "CYNDI - Chip8 | OPS: 800", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*WIN_SCALE, 32*WIN_SCALE, SDL_WINDOW_SHOWN );
  SDL_Renderer* gameRenderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(gameRenderer,SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(gameRenderer,backgroundRGB[0],backgroundRGB[1],backgroundRGB[2],255);
  SDL_Event event;
  bool quit = false;
  SDL_Rect *pixel = new SDL_Rect;
  pixel->x = 0;
  pixel->y = 0;
  pixel->w = WIN_SCALE;
  pixel->h = WIN_SCALE;
  int opsPerSec = 800;
  double msecPerOp = 1000.0 / 800;
  int delayTicks = 0;
  int delayDeltaTicks = 0;
  double sixtyHertz = 1000.0 / 60.0; //miliseonds
  int startCycleTicks = 0;
  int cycleDelta = 0;
  bool drawFrane = true;

  //main loop
  while(!quit) {
    //timing of cycle
    startCycleTicks = SDL_GetTicks();

    //draw a frame unless told otherwise
    drawFrane = true;

    //input handling
    while(SDL_PollEvent(&event) != 0) {
      switch(event.type) {
        case SDL_QUIT:
          quit = true;
          break;
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
          const uint8_t *keyState = SDL_GetKeyboardState(NULL);
          bool keys[16];
          //this is a default mapping. May want to change
          keys[0] = keyState[SDL_SCANCODE_X];
          keys[1] = keyState[SDL_SCANCODE_1];
          keys[2] = keyState[SDL_SCANCODE_2];
          keys[3] = keyState[SDL_SCANCODE_3];
          keys[4] = keyState[SDL_SCANCODE_Q];
          keys[5] = keyState[SDL_SCANCODE_W];
          keys[6] = keyState[SDL_SCANCODE_E];
          keys[7] = keyState[SDL_SCANCODE_A];
          keys[8] = keyState[SDL_SCANCODE_S];
          keys[9] = keyState[SDL_SCANCODE_D];
          keys[10] = keyState[SDL_SCANCODE_Z];
          keys[11] = keyState[SDL_SCANCODE_C];
          keys[12] = keyState[SDL_SCANCODE_4];
          keys[13] = keyState[SDL_SCANCODE_R];
          keys[14] = keyState[SDL_SCANCODE_F];
          keys[15] = keyState[SDL_SCANCODE_V];
          cpu.setKeys(keys);
          if(keyState[SDL_SCANCODE_RIGHT]) {
            opsPerSec+=100;
            msecPerOp = 1000.0 / opsPerSec;
            char title[256];
            sprintf(title,"CYNDI - Chip8 | OPS %d",opsPerSec);
            SDL_SetWindowTitle(window,title);
          }
          if(keyState[SDL_SCANCODE_LEFT]) {
            opsPerSec-=100;
            msecPerOp = 1000.0 / opsPerSec;
            char title[256];
            sprintf(title,"CYNDI - Chip8 | OPS %d",opsPerSec);
            SDL_SetWindowTitle(window,title);
          }
          break;
        }
        default:
          break;
      }
    }

    //execute next instruction
    switch(cpu.executeOp()) {
      case chip_oob:
        if(DEBUG_MODE)
          cpu.debug("Stopped execution due to bad address. Check I\n");
      case chip_exit:
        quit = true;
        break;
      case chip_skipDraw:
        drawFrane = false;
        break;
      case chip_normal:
      default:
        break;
    }

    //chip8 has 2 60Hz timers
    delayDeltaTicks = SDL_GetTicks();
    if( delayDeltaTicks - delayTicks > sixtyHertz) {
      cpu.timerTick();
      delayTicks = delayDeltaTicks;
    }

    if(drawFrane) {
      //clear screen
      SDL_SetRenderDrawColor(gameRenderer,backgroundRGB[0],backgroundRGB[1],backgroundRGB[2],255);
      SDL_RenderClear(gameRenderer);

      //draw active pixels
      int cur_pix;
      for(int i = 0; i < PIX_COUNT; i++) {
        cur_pix = cpu.getPixel(i);
        if(cur_pix) {
          SDL_SetRenderDrawColor(gameRenderer,(cur_pix >> 16) & 0xFF, (cur_pix >> 8) & 0xFF, cur_pix & 0xFF,80);
          pixel->x = (i % PIX_WIDTH) * WIN_SCALE;
          pixel->y = (i / PIX_WIDTH) * WIN_SCALE;
          SDL_RenderFillRect(gameRenderer,pixel);
        }
      }
    }

    //display screen, wait
    SDL_RenderPresent(gameRenderer);
    cycleDelta = SDL_GetTicks() - startCycleTicks;
    if (cycleDelta < msecPerOp)
      SDL_Delay(msecPerOp - cycleDelta);
  }

  //dump CPU and cleanup
  if(DEBUG_MODE)
    cpu.dumpCpu();
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