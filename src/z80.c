#include "z80.h"
#include "common.h"
cpu_t z80;

cpu_t step() {
  // z80.PC = 0x100;
  runOpcode(&z80);
}
inline int readMem(uint16_t addr) {
  if (addr == 0x0 && addr <= 0x99) {
    return ram[addr];
  } else if (addr >= 0x100 && addr <= 0x2300) {
    return zexall[addr - 0x100];
  } else if (addr >= 0x2301 && 0xFFFF) {
    return ram[addr - 0x2301];
  } else
    return -1;
}
inline void writeMem(uint8_t data, uint16_t addr) {
  if (addr == 0x0 && addr <= 0x99) {
    ram[addr] = data;
  } else if (addr >= 0x100 && addr <= 0x2300) {
    zexall[addr - 0x100] = data;
  } else if (addr >= 0x2301 && 0xFFFF) {
    ram[addr - 0x2301] = data;
  }
}
