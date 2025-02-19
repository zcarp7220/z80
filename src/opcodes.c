#include "common.h"
#include "z80.h"
#define LD(A, B) (*(B) = *(A))
void setFlag(cpu_t *z80, int flag) { *z80.F |= 1 << flag; }
void clearFlag(cpu_t *z80, int flag) { *z80.F &= ~(1 << flag); }
void runOpcode(cpu_t *z80) {
  switch (*z80.PC) {
  case 0xCB:

  case 0xED:

  case 0xDD:

  case 0xFD:

  default:
  }
}
