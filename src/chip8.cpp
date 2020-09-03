#include "chip8.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
extern bool DEBUG_MODE;

Chip8::Chip8() {
  //clear everything to 0
  for(int i = 0; i < 4096; i++)
    memory[i] = 0;
  for(int i = 0; i < 16; i++) {
    V[i] = 0;
    stack[i] = 0;
    keys[i] = false;
  }
  mem_reg = 0;
  delay = 0;
  sound = 0;
  sp = 0;
  pc = 0;
  opcode = 0;
  opCount = 0;
  for(int i = 0; i < PIX_COUNT; i++) {
    board[i] = false;
  }
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

  int x_code, y_code;
  x_code = opcode & 0x0F00;
  y_code = opcode & 0x00F0;
  x_code>>=8;
  y_code>>=4;
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
      pc = V[x_code] == (opcode & 0x00FF) ? pc+4 : pc+2;
      break;
    case 0x4000:
      //4XNN - skip next if VX != NN
      pc = V[x_code] != (opcode & 0x00FF) ? pc+4 : pc+2;
      break;
    case 0x5000:
      //5XY0 - skip next if VX == VY
      pc = V[x_code] == V[y_code] ? pc+4 : pc+2;
      break;
    case 0x6000:
      //6XNN - set VX to NN
      V[x_code] = opcode & 0x00FF;
      pc+=2;
      break;
    case 0x7000:
      //7XNN - add NN to VX
      V[x_code] = (int8_t)(V[x_code] + opcode & 0x00FF);
      pc+=2;
      break;
    case 0x8000:
      switch (opcode & 0x000F) {
        case 0:
          //8XY0 - Set VX = VY
          V[x_code] = V[y_code];
          break;
        case 1:
          //8XY1 - Set VX = VX OR VY
          V[x_code] = V[x_code] | V[y_code];
          break;
        case 2:
          //8XY2 - Set VX = VX AND VY
          V[x_code] = V[x_code] & V[y_code];
          break;
        case 3:
          //8XY3 - Set VX = VX XOR VY
          V[x_code] = V[x_code] ^ V[y_code];
          break;
        case 4:
          //8XY4 - Set VX = VX + XY, set VF as cary
          V[15] = V[x_code] + V[y_code] > 0xFF ? 1 : 0;
          V[x_code]+=V[y_code];
          break;
        case 5:
          //8XY5 - Set VX = VX - VY, set VF to 0 if borrow
          V[15] = V[x_code] > V[y_code] ? 1: 0;
          V[x_code] = (uint8_t)(V[x_code] - V[y_code]);
          break;
        case 6:
          //8XY6 - Set VX = VY >> 1, store LSB of VY in VF
          //this op code seems to be contested
          //this was an undoc'd opcode in the original spec.
          V[x_code] = V[y_code] >> 1;
          V[15] = V[y_code] & 0x0001 == 1 ? 1 : 0;
          break;
        case 7:
          //8XY7 - Set VX = VY - VX
          V[15] = V[x_code] > V[y_code] ? 0 : 1;
          V[x_code] = (uint8_t)(V[y_code] - V[x_code]);
          break;
        case 0xe:
          //8XYE - Set VX = VY << 1, store MSB of VY in VF
          //this is also a contested op code.
          //this was an undoc'd opcode in the original spec
          V[15] = V[y_code] & 0x8000 == 0x8000 ? 1 : 0;
          V[x_code] = V[y_code] << 1;
          break;
      }
      pc+=2;
      break;
    case 0x9000:
      //9XY0 - Skip next instruction if VX != VY
      pc += V[x_code] != V[y_code] ? 4 : 2;
      break;
    case 0xa000:
      //ANNN - Set I = NNN
      mem_reg = opcode & 0x0FFF;
      pc += 2;
      break;
    case 0xb000:
      //BNNN - Jump to V0 + NNN
      pc = V[0] + (opcode & 0x0FFF);
      break;
    case 0xc000:
      //CXNN - Set VX = random number 0 to 255 masked with NN
      V[x_code] = (std::rand() % 256) & (opcode & 0x00FF);
      pc += 2;
      break;
    case 0xd000:
      //DXYN - Draw N byte sprite in I at (VX,VY). If any pixels turned off, set VF = 1
      V[15] = 0;
      for(int i = 0; i < (opcode & 0x000F); i++) {
        for(int j = 0; j < 8; j++) {
          int cur_pixel = ((V[y_code] + i) % PIX_HEIGHT) * PIX_WIDTH + (V[x_code] + j) % PIX_WIDTH;
          if ((board[cur_pixel] == true) && ((memory[mem_reg+i] & (0x80 >> j)) != 0))
            V[15] = 1;
          //this is a rather long way of writing a logical XOR
          //seems like it should be doable in 1 line, but C++ aint havin it rn
          if(board[cur_pixel]) {
            if(memory[mem_reg+i] & (0x80 >> j))
              board[cur_pixel] = false;
          } else {
            if(memory[mem_reg+i] & (0x80 >> j))
              board[cur_pixel] = true;
          }
        }
      }
      pc += 2;
      break;
    case 0xE000:
      if((opcode & 0x00FF) == 0x9E) {
        //EX9E - Skip next if key in Vx is pressed
        pc += keys[(V[x_code] & 0x000F)] ? 4 : 2;
      } else if((opcode & 0x00FF) == 0xA1) {
        //EXA1 - Skip next if key in Vx is NOT pressed
        pc += keys[(V[x_code] & 0x000F)] ? 2 : 4;
      } else {
        std::cout << "Bad opcode.\n";
        if(DEBUG_MODE)
          debug("BAD OPCODE");
      }
      break;
  }

  if(DEBUG_MODE) {
    debug("--\n");
  }
  return chip_normal;
};

bool Chip8::getPixel(int pix) {
  return board[pix];
};

void Chip8::setKeys(bool *newKeys) {
  //this function is stupid
  //the only reason it exists is to follow "encapsulation"
  //and good OOP standards.
  for(int i = 0; i < 16; i++)
    keys[i] = newKeys[i];
  return;
}

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
  ss.str("");
  ss << "PC: 0x" << std::hex << (int)pc << std::dec << "\n";
  debug(ss.str());
  ss.str("");
  ss << "I: 0x" << std::hex << (int)mem_reg << std::dec << "\n";
  debug(ss.str());
  ss.str("");
  ss << "SP: 0x" << std::hex << (int)sp << std::dec << "\n";
  debug(ss.str());
  ss.str("");
  ss << "Delay Timer: 0x" << std::hex << (int)delay << std::dec << "\n";
  debug(ss.str());
  ss.str("");
  ss << "Sound Timer: 0x" << std::hex << (int)sound << std::dec << "\n";
  debug(ss.str());
  debug("\n\nFinal RAM dump\n");
  for(int i = 0; i < 4096; i+=8) {
    ss.str("");
    ss << "0x" << std::hex << i << " ";
    for(int j = 0; j < 8; j++)
      ss << (int)memory[i+j] << " ";
    ss << "\n";
    debug(ss.str());
  }
  return;
};