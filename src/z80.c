#include "z80.h"
#include "common.h"
cpu_t z80;

cpu_t step() {
  // z80.PC = 0x100;
  runOpcode(&z80);
}
inline int readMem(uint16_t addr) { return ram[addr]; }
inline void writeMem(uint8_t data, uint16_t addr) { ram[addr] = data; }
