#ifndef _CHIP_8_
#define _CHIP_8_
#include <cstdint>

class Chip8 {
	public:
		void test();
	private:
		int8_t memory[4096]; //4kb of memory
		int8_t V[16]; //16 8 bit registers
		int16_t mem_reg;
		int8_t delay;
		int8_t sound;
		int8_t sp; //stack pointer
		int16_t pc; //program counter
		int16_t stack[16]; //stack
};

#endif