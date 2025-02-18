#include "z80.h"
struct cpu z80;
void step(u_int8_t opcode[3]) {
  z80.BC = z80.B << 8 | z80.C;
  z80.DE = z80.D << 8 | z80.E;
  z80.HL = z80.H << 8 | z80.L;
  if (opcode[0] == 0xCB) {
    switch (opcode[0]) {}
  } else {
    switch (opcode[0]) {
    case 0x0:
      z80.PC += 1;
      break;
    case 0x01:
      z80.C = opcode[1];
      z80.B = opcode[2];
      z80.PC += 3;
      break;
    case 0x02:
    }
  }
}
