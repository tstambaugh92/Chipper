#ifndef _CHIP_8_
#define _CHIP_8_
#include <cstdint>
#include <fstream>
#include <list>
#define PIX_WIDTH 64
#define PIX_HEIGHT 32
#define PIX_COUNT 64*32

struct spriteColor {
  char location[2];
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint16_t add;
};

enum returnCodes {
  chip_normal,
  chip_exit,
  chip_oob
};

class Chip8 {
  public:
    ~Chip8();
    Chip8();
    int loadROM(char* filename);
    int executeOp();
    void timerTick();
    int getPixel(int);
    void setKeys(bool *);
    void debug(std::string);
    void dumpCpu();
  private:
    uint8_t memory[4096]; //4kb of memory
    uint8_t V[16]; //16 8 bit registers
    uint16_t mem_reg; //known as "I" in Chip8 terms. Renamed since i is common for loops
    uint8_t delay; // delay timer
    uint8_t sound; //sound timer
    uint8_t sp; //stack pointer
    uint16_t pc; //program counter
    uint16_t stack[16]; //stack
    uint16_t opcode;
    bool keys[16];
    int board[PIX_COUNT];
    int opCount;
    bool customControls;
    bool customColors;
    std::list<spriteColor> colorsList;
    std::list<spriteColor>::iterator it;

    std::ofstream log;
};

#endif