#include "chip8.h"
#include <iostream>
#include <fstream>

Chip8::Chip8(bool *screen) {
  //clear everything to 0
  for(int i = 0; i < 4096; i++)
    memory[i] = 0;
  for(int i = 0; i < 16; i++) {
    V[i] = 0;
    stack[i] = 0;
  }
  mem_reg = 0;
  delay = 0;
  sound = 0;
  sp = 0;
  pc = 0;
  opcode = 0;
  board = screen;

  //font data
  int font[] = {
    0xf0,0x90,0x90,0x90,0xf0, //0
    0x20,0x60,0x20,0x20,0x70, //1
    0xf0,0x10,0xf0,0x80,0xf0, //2
    0xf0,0x10,0xf0,0x10,0xf0, //3
    0x90,0x90,0xf0,0x10,0x10, //4
    0xf0,0x80,0xf0,0x10,0xf0, //5
    0xf0,0x80,0xf0,0x90,0xf0, //6
    0xf0,0x10,0x20,0x40,0x40, //7
    0xf0,0x90,0xf0,0x90,0xf0, //8
    0xf0,0x90,0xf0,0x10,0xf0, //9
    0xf0,0x90,0xf0,0x90,0x90, //A
    0xe0,0x90,0xe0,0x90,0xe0, //B
    0xf0,0x80,0x80,0x80,0xf0, //C
    0xe0,0x90,0x90,0x90,0xe0, //D
    0xf0,0x80,0xf0,0x80,0xf0, //E
    0xf0,0x80,0xf0,0x80,0x80 //F
    };
  for(int i = 0; i < 80; i++)
    memory[i] = (int8_t)font[i];
};

int Chip8::loadROM(char* filename) {
  std::ifstream gameROM;
  //ROM opens at end of file to get filesize. Returns to start of file
  gameROM.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if(!gameROM.good())
    return -1;
  int fileSize = gameROM.tellg();
  gameROM.seekg(0);
  gameROM.read((char *)&memory[200],fileSize);
  gameROM.close();
  return 0;
};

void Chip8::putFont(int index, bool* board, int pos) {
    //this will be modified for sprites later
    //note that this only puts 4 bits, since all fonts
    //are 4 bits wide. Normal sprites will need 8
    for(int j = 0; j < 5; j++) {
      for(int k = 0; k < 4; k++) {
        board[pos + j*64 + k] = (bool) (memory[index*5 + j] & (0x80 >> k));
      }
    }
};