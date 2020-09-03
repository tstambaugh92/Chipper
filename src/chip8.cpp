#include "chip8.h"
#include <iostream>
#include <fstream>
#include <sstream>
extern bool DEBUG_MODE;

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
  opCount = 0;
  board = screen;
  if(DEBUG_MODE) {
    std::cout << "Creating log file\n";
    log.open("log.txt",std::ios::trunc);
    if(!log.good()) {
      std::cout << "Error opening log file. Debug disabled\n";
      DEBUG_MODE = false;
    }
  }

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
    memory[i] = (uint8_t)font[i];
};

Chip8::~Chip8() {
  if(log.good())
    log << "\n";
    log.close();
};

int Chip8::loadROM(char* filename) {
  std::ifstream gameROM;
  if(DEBUG_MODE) {
    std::string _filename(filename);
    _filename = "Opening ROM " + _filename + "\n";
    debug(_filename);
  }

  //ROM opens at end of file to get filesize. Returns to start of file
  gameROM.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if(!gameROM.good())
    return -1;
  int fileSize = gameROM.tellg();
  gameROM.seekg(0);
  gameROM.read((char *)&(memory[0x200]),fileSize);
  gameROM.close();
  pc = 0x200; //default starting area for Chip8 games
  std::cout << "ROM opened\n";

  if(DEBUG_MODE)
    debug("ROM opened and loaded sucsessfully.\n");
  return 0;
};

int Chip8::executeOp(uint16_t testOp) {
  opCount++;
  if(testOp == 0) {
    opcode = memory[pc];
    opcode <<=8;
    opcode += memory[pc+1];
  } else {
    opcode = testOp;
  }

  if(DEBUG_MODE) {
    std::stringstream ss;
    ss << std::dec << opCount;
    debug(ss.str() + ": ");
    ss.str("");
    ss << "0x" << std::hex << pc;
    debug("pc - " + ss.str() + ", ");
    ss.str("");
    ss << "0x" << std::hex << opcode;
    debug("opcode - " + ss.str() + " ");
  }

  int temp, temp2;
  switch(opcode & 0xF000) {
    case 0x0000:
      if(opcode == 0x00E0) {
        //0x00E0 - clear screen
        for(int i = 0; i < PIX_COUNT; i++)
          board[i] = false;
        pc+=2;
      } else if(opcode == 0x00EE) {
        //0x00EE - return from sub
        sp--;
        pc = stack[sp];
      } else {
        //machine code possible here. NOP for now
        if (opcode == 0x0000) {
          std::cout << "ending game\n";
          if(DEBUG_MODE) {
            debug("+\nEnding game\n");
          }
          return chip_exit;
        }
      }
      break;
    case 0x1000:
      //1NNN - jump to NNN
      pc = opcode & 0x0FFF;
      break;
    case 0x2000:
      //2NNN - call sub
      pc++;
      stack[sp] = pc;
      sp++;
      pc = opcode & 0x0FFF;
      break;
    case 0x3000:
      //3XNN - skip next if VX == NN
      temp = opcode & 0x0F00;
      temp >>= 8;
      pc = V[temp] == (opcode & 0x00FF) ? pc+4 : pc+2;
      break;
    case 0x4000:
      //4XNN - skip next if VX != NN
      temp = opcode & 0x0F00;
      temp >>= 8;
      pc = V[temp] != (opcode & 0x00FF) ? pc+4 : pc+2;
      break;
    case 0x5000:
      //5XY0 - skip next if VX == VY
      temp = opcode & 0x0F00;
      temp2 = opcode & 0x00F0;
      temp >>= 8;
      temp2 >>= 4;
      pc = V[temp] == V[temp2] ? pc+4 : pc+2;
      break;
    case 0x6000:
      //6XNN - set VX to NN
      temp = opcode & 0x0F00;
      temp >>= 8;
      V[temp] = opcode & 0x00FF;
      pc+=2;
      break;
    case 0x7000:
      //7XNN - add NN to VX
      temp = opcode & 0x0F00;
      V[temp] = (int8_t)(V[temp] + opcode & 0x00FF);
      pc+=2;
      break;
    case 0x8000:
      temp = opcode & 0x0F00;
      temp2 = opcode & 0x00F0;
      temp>>=8;
      temp2>>=4;
      switch (opcode & 0x000F) {
        case 0:
          //8XY0 - Set VX = VY
          V[temp] = V[temp2];
          break;
        case 1:
          //8XY1 - Set VX = VX OR VY
          V[temp] = V[temp] | V[temp2];
          break;
        case 2:
          //8XY2 - Set VX = VX AND VY
          V[temp] = V[temp] & V[temp2];
          break;
        case 3:
          //8XY3 - Set VX = VX XOR VY
          V[temp] = V[temp] ^ V[temp2];
          break;
        case 4:
          //8XY4 - Set VX = VX + XY, set VF as cary
          V[15] = V[temp] + V[temp2] > 0xFF ? 1 : 0;
          V[temp]+=V[temp2];
          break;
        case 5:
          //8XY5 - Set VX = VX - VY, set VF to 0 if borrow
          V[15] = V[temp] > V[temp2] ? 1: 0;
          V[temp] = (uint8_t)(V[temp] - V[temp2]);
          break;
      }
      pc+=2;
      break;
  }

  if(DEBUG_MODE) {
    debug("--\n");
  }
  return chip_normal;
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

void Chip8::debug(std::string dbString) {
  log << dbString;
  return;
};

void Chip8::dumpCpu() {
  std::stringstream ss;
  debug("\n\nFinal CPU dump:\n");
  for(int i = 0; i < 16; i ++) {
    ss.str("");
    ss << "V" << std::hex << i  << std::dec << ": 0x" << std::hex << (int)V[i] << std::dec<< std::endl;
    debug(ss.str());
  }
  return;
};