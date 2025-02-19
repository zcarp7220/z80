#include "z80.h"
struct cpu z80;
void step(u_int8_t opcode[3]) {
  if (opcode[0] == 0xCB) {
    switch (opcode[0]) {}
  } else {
    switch (opcode[0]) {
    case 0x0:
      z80.PC += 1;
      break;
    case 0x01:
      z80.BC = opcode[1] | opcode[2] << 8;
      z80.PC += 3;
      break;
    case 0x02:
      z80.A = memRead(z80.BC);
      z80.PC += 2;
      break;
    case 0x03:
      z80.BC++;
      z80.PC += 1;
      break;
    case 0x04:
      z80.B++;
      z80.PC += 1;
      break;
    case 0x05:
      z80.B--;
      z80.PC += 1;
      break;
    }
  }
}
