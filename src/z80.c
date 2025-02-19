#include "z80.h"
struct cpu z80;

void step() {
  z80.PC = 0x100;
  runOpcode(&z80);
}
