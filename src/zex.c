#include "common.h"
#include "z80.h"
void zexStep() {
  z80.PC = 0x100;
  writeMem(0x7, 0xC9);
  for (;;) {
    cpuStep();
    if (z80.PC == 5) {
      if (z80.C == 2) {
        printf("%c\n", z80.E);
      } else if (z80.C == 9) {
        int i = z80.DE;
        while (readMem(i) != 0x24) {
          printf("%c", readMem(i));
          i++;
        }
      }
    }
  }
}
