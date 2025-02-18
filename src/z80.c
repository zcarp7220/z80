#include "z80.h"
struct cpu z80;
void step(u_int8_t opcode[3]) {
  BCOperatons(true);
  DEOperatons(true);
  HLOperatons(true);
  if (opcode[0] == 0xCB) {
    switch (opcode[0]) {}
  } else {
    switch (opcode[0]) {
    case 0x0:
      z80.PC += 1;
      break;
    case 0x01:
      z80.BC = opcode[1] | opcode[2] << 8;
      BCOperatons(true);
      z80.PC += 3;
      break;
    case 0x02:
      z80.A = loadMemory(z80.BC);
      z80.PC += 2;
      break;
    case 0x03:
      z80.BC++;
      BCOperatons(false);
      z80.PC += 1;
      break;
    case 0x04:
      z80.B++;
      z80.PC += 1;
      break;
    }
  }
}
void BCOperatons(bool BCMerge) {
  if (BCMerge) {
    z80.BC = z80.B << 8 | z80.C;
  } else {
    z80.B = z80.BC & 0xF0 >> 0xF;
    z80.C = z80.BC & 0x0F;
  }
  return;
}
void DEOperatons(bool DEMerge) {
  if (BCMerge) {
    z80.DE = z80.D << 8 | z80.E;
  } else {
    z80.D = z80.DE & 0xF0 >> 0xF;
    z80.E = z80.DE & 0x0F;
  }
  return;
}
void HLOperatons(bool HLMerge) {
  if (BCMerge) {
    z80.HL = z80.H << 8 | z80.L;
  } else {
    z80.H = z80.HL & 0xF0 >> 0xF;
    z80.L = z80.HL & 0x0F;
  }
  return;
}
