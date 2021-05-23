#include "chip8.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
extern bool DEBUG_MODE;
extern bool FIND_MODE;

Chip8::Chip8() {
  //clear everything to 0
  memset(memory,0,4096);
  for(int i = 0; i < 16; i++) {
    //memset *may* be better here
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
  customControls = false;
  customColors = true;
  memset(board,0,PIX_COUNT*sizeof(int));
  if(DEBUG_MODE) {
    std::cout << "Creating log file\n";
    log.open("log.txt",std::ios::trunc);
    if(!log.good()) {
      std::cout << "Error opening log file. Debug disabled\n";
      DEBUG_MODE = false;
    }
  }

  //font data
  uint8_t font[] = {
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
  //load font in memory
  memcpy(memory,font,80);
};

Chip8::~Chip8() {
  if(log.good())
    log << "\n";
    log.close();
};

int Chip8::loadROM(char* filename) {
  std::ifstream gameROM;
  std::ifstream colorFile;
  std::string _filename(filename);

  if(DEBUG_MODE) {
    debug("Opening ROM " + _filename + "\n");
  }

  //ROM opens at end of file to get filesize. Returns to start of file
  gameROM.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if(!gameROM.good())
    return -1;
  int fileSize = gameROM.tellg();
  if(fileSize > 0xE00) {
    std::cout << "ROM too large.\n";
    if(DEBUG_MODE)
      debug("ROM too large.\n");
    gameROM.close();
    return -1;
  }

  int nameIndex = 0;
  for(int i = _filename.size() - 1; _filename[i] != '\\'; --i ) {
    nameIndex = i;
  }

  //searh for a .clr file with same name
  std::string _colorfilename = "..\\colors\\" + _filename.substr(nameIndex);
  _colorfilename[_colorfilename.size() - 2] = 'l';
  _colorfilename[_colorfilename.size() - 1] = 'r';
  _colorfilename[_colorfilename.size()] = 0;
  if(DEBUG_MODE)
    debug("Trying color file " + _colorfilename);
  colorFile.open(_colorfilename.c_str(),std::ios::in | std::ios::binary | std::ios::ate);
  if(colorFile.good()) {
    if(DEBUG_MODE)
      debug("Custom colors found.\n");
    customColors = true;
  } else {
    if(DEBUG_MODE)
      debug("No custom colors\n");
  }

  //read and save colors from clr file
  spriteColor tempColor;
  tempColor.location[0] = 0;
  tempColor.location[1] = 0;
  tempColor.r = 0;
  tempColor.g = 0;
  tempColor.b = 0;
  int numOfColors = ((int)colorFile.tellg()) / 5;
  if(numOfColors < 2) {
    //first two colors should be a custom background and then default color
    customColors = false;
    debug("Need at least two colors in clr file.\n");
    debug("Custom Color mode aborted.\n");
  } else {
    debug(std::to_string(numOfColors) + " colors\n");
    colorFile.seekg(0);
    for(int i = 0; i < numOfColors; i++) {
      colorFile.read((char *)&tempColor,5);
      tempColor.add = (*((uint8_t *)(tempColor.location)) << 8) + *((uint8_t *)(&(tempColor.location[1])));
      if(DEBUG_MODE) {
        debug("Color for ");
        log << std::hex;
        if(i==0) {
          debug("background ");
        } else if (i==1) {
          debug("default color ");
        } else {
          debug("0x");
          debug(tempColor.add);
        }
        debug(" R:0x");
        debug(tempColor.r);
        debug(" G:0x");
        debug(tempColor.g);
        debug(" B:0x");
        debug(tempColor.b);
        log << std::dec;
        debug("\n");
      }
      colorsList.push_back(tempColor);
    }
  }
  colorFile.close();
  it = colorsList.begin();

  gameROM.seekg(0);
  gameROM.read((char *)&(memory[0x200]),fileSize);
  gameROM.close();
  pc = 0x200; //default starting area for Chip8 games
  std::cout << "ROM opened\n";

  if(DEBUG_MODE)
    debug("ROM opened and loaded sucsessfully.\n");
  return 0;
};

int Chip8::executeOp() {
  bool drawFrame = false;
  opCount++;
  if(pc >= 4096) {
    std::cout << "PC is out of bounds\n";
    if(DEBUG_MODE)
      debug("PC is OOB. See CPU dump.\n");
    return chip_oob;
  }

  //loading each byte manually rather than 2 bytes
  //prevents endian errors. Chip8 is big endian
  opcode = memory[pc];
  opcode <<=8;
  opcode += memory[pc+1];

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
          //treating 0x0000 as end game
          std::cout << "ending game\n";
          if(DEBUG_MODE) {
            debug("+\nEnding game normally\n");
          }
          return chip_exit;
        } else {
          std::cout << "Bad opcode. NOP\n";
          if(DEBUG_MODE)
            debug("BAD OPCODE");
          pc+=2;
        }
      }
      break;
    case 0x1000:
      //1NNN - jump to NNN
      pc = opcode & 0x0FFF;
      break;
    case 0x2000:
      //2NNN - call sub
      pc+=2;
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
          V[15] = V[x_code] & 0x01 == 1 ? 1 : 0;
          V[x_code] = (V[x_code] >> 1);
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
          V[15] = V[x_code] & 0x80 == 0x8000 ? 1 : 0;
          V[x_code] = (V[x_code] << 1);
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
    case 0xd000: {
      //DXYN - Draw N byte sprite in I at (VX,VY). If any pixels turned off, set VF = 1
      int draw_color = 0x0000FF00;
      if(customColors) {
        it = colorsList.begin();
        it++;
        draw_color = (it->r << 16) | (it->g << 8) | (it->b); //2nd color default
        it++;
        for (int i = 2; i < colorsList.size(); i++) {
          if (it->add == mem_reg) {
            draw_color = (it->r << 16) | (it->g << 8) | (it->b);
            break;
          }
          it++;
        }
      }

      if(mem_reg + (opcode & 0x000F) >= 4096) {
        std::cout << "Attempt to access out of bounds memory.";
        if(DEBUG_MODE)
          debug("BAD MEMORY\n");
        return chip_oob;
      } 
      V[15] = 0;
      if(FIND_MODE) //print the address of a sprite. useful for custom colors
        std::cout << "Sprite at I 0x" << std::hex << mem_reg << " " << opcode << std::dec << "\n";
      for(int i = 0; i < (opcode & 0x000F); i++) {
        for(int j = 0; j < 8; j++) {
          int cur_pixel = ((V[y_code] + i) % PIX_HEIGHT) * PIX_WIDTH + (V[x_code] + j) % PIX_WIDTH;
          if ((board[cur_pixel] != 0) && ((memory[mem_reg+i] & (0x80 >> j)) != 0))
            V[15] = 1;
          //this is a rather long way of writing a logical XOR
          //seems like it should be doable in 1 line, but C++ aint havin it rn
          if(board[cur_pixel]) {
            if(memory[mem_reg+i] & (0x80 >> j))
              board[cur_pixel] = 0;
          } else {
            if(memory[mem_reg+i] & (0x80 >> j)) {
              board[cur_pixel] = draw_color;
              drawFrame = true;
            }
          }
        }
      }
      pc += 2;
      break;
    }
    case 0xE000:
      switch(opcode & 0x00FF) {
        case 0x9E:
          //EX9E - Skip next if key in Vx is pressed
          pc += keys[(V[x_code] & 0x000F)] ? 4 : 2;
          break; 
        case 0xA1:
          //EXA1 - Skip next if key in Vx is NOT pressed
          pc += keys[(V[x_code] & 0x000F)] ? 2 : 4;
          break;
        default:
          //bad code. NOP
          std::cout << "Bad opcode.\n";
          if(DEBUG_MODE)
            debug("BAD OPCODE");
          pc+=2;
          break;
      }
      break;
    case 0xF000:
      switch(opcode & 0x00FF) {
        case 0x07:
          //FX07 - Store delay timer in VX
          V[x_code] = delay;
          pc+=2;
          break;
        case 0x0A:
          //FX0A - Wait for keypress and store in Vx
          //this is more like a system interupt.
          //will not progress past this opcode until keypress
          for(int i = 0; i < 16; i++) {
            if(keys[i]) {
              V[x_code] = i;
              pc+=2;
              break;
            }
          }
          break;
        case 0x15:
          //FX15 - Set delay timer = VX
          delay = V[x_code];
          pc+=2;
          break;
        case 0x18:
          //FX18 - Set sound timer = VX
          sound = V[x_code];
          pc+=2;
          break;
        case 0x1E:
          //FX1E - Set I = I + VX
          mem_reg += V[x_code];
          pc+=2;
          break;
        case 0x29:
          //FX29 - Load font of number in VX into I
          if(V[x_code] > 0xF) {
            std::cout << "Attempt to load bad font\n";
            if(DEBUG_MODE)
              debug("BAD FONT");
          }
          mem_reg = V[x_code] * 5;
          pc+=2;
          break;
        case 0x33:
          //FX33 - Load BCD of VX into I, I+1, I+2
          if(mem_reg+2 >= 4096) {
            std::cout << "Attempt to access out of bounds memory.";
            if(DEBUG_MODE)
              debug("BAD MEMORY\n");
            return chip_oob;
          }
          memory[mem_reg] = (V[x_code] / 100);
          memory[mem_reg+1] = ((V[x_code] % 100) / 10);
          memory[mem_reg+2] = ((V[x_code] % 100) % 10);
          pc+=2;
          break;
        case 0x55:
          //FX55 - Store V0 through VX at I to I+X
          if(mem_reg+x_code >= 4096) {
            std::cout << "Attempt to access out of bounds memory.";
            if(DEBUG_MODE)
              debug("BAD MEMORY\n");
            return chip_oob;  
          }
          for(int i = 0; i <= x_code; i++) {
            memory[mem_reg+i] = V[i];
          }
          pc+=2;
          break;
        case 0x65:
          //FX65 Load V0 to VX from I to I+X
          if(mem_reg+x_code >= 4096) {
            std::cout << "Attempt to access out of bounds memory.";
            if(DEBUG_MODE)
              debug("BAD MEMORY\n");
            return chip_oob;  
          }
          for(int i = 0; i <= x_code; i++) {
            V[i] = memory[mem_reg+i];
          }
          pc+=2;
          break;
        default:
          //Bad Opcode. NOP
          std::cout << "Bad Opode\n";
          if(DEBUG_MODE)
            debug("BAD OPCODE");
          pc+=2;
          break;
      }
      break;
  }

  if(DEBUG_MODE) {
    debug("--\n");
  }
  if(drawFrame) {
    return chip_normal;
  } else {
    return chip_skipDraw;
  }
};

void Chip8::timerTick() {
  if(delay > 0)
    delay--;
  if(sound > 0)
    sound--;
  return;
}

int Chip8::getPixel(int pix) {
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
  ss.str("");
  debug("\n\nSprite Dump\n");
  it = colorsList.begin();
  for(int i = 0; i<colorsList.size(); i++) {
    ss.str("");
    ss << "Sprit Add: " << std::hex << (int)it->add << "\nR:" << (int) it->r << "\nG:" << it->g << "\nB:" << (int) it->b << std::dec << "\n\n";
    it++;
    debug(ss.str());
  }

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

bool Chip8::areCustomColors() {
  return customColors;
};

void Chip8::getBackgroundRGB(int rgb[3]) {
  //the first custom color is always the background
  it = colorsList.begin();
  rgb[0] = it->r;
  rgb[1] = it->g;
  rgb[2] = it->b;
  return;
};

void Chip8::debug(std::string dbString) {
  log << dbString;
  return;
};

void Chip8::debug(int str) {
  log << str;
  return;
};