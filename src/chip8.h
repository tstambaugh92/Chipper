#ifndef _CHIP_8_
#define _CHIP_8_
#include <cstdint>
#include <fstream>
#include <string>
#define PIX_WIDTH 64
#define PIX_HEIGHT 32
#define PIX_COUNT 64*32

enum returnCodes {
  chip_normal,
  chip_exit
};

class Chip8 {
  public:
    ~Chip8();
    Chip8(bool* screen);
    int loadROM(char* filename);
    void putFont(int index, bool* board, int pos);
    int executeOp(uint16_t testOp);
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
    bool* board;
    int opCount;
    std::ofstream log;
};

#endif