#include "z80.h"
cpu_t z80;

void step() {
  z80.PC = 0x100;
  runOpcode(&z80);
}
