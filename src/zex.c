#include "common.h"
#include "z80.h"
void zexStep() {
  z80.AF = 0xFFFF;
//  z80.BC = 0xFFFF;
//  z80.DE = 0xFFFF;
//  z80.HL = 0xFFFF;
//  z80.AFp = 0xFFFF;
//  z80.BCp = 0xFFFF;
//  z80.DEp = 0xFFFF;
//  z80.HLp = 0xFFFF; 
  z80.PC = 0x100;
  z80.cycles = 1;

  for (;;) {   
    fprintf(stderr, "PC: 0x%X, 3 bytes after PC: 0x%X 0x%X 0x%X\n", z80.PC, readMem(z80.PC), readMem(z80.PC + 1), readMem(z80.PC + 2));
    fprintf(stderr, "Cycles: %lld A: 0x%02X  BC: 0x%02X %02X  DE: 0x%02X %02X  HL: 0x%02X %02X\n",
      z80.cycles, z80.A,
      z80.B, z80.C,  
      z80.D, z80.E,  
      z80.H, z80.L);
      cpuStep();
    if(z80.PC == 0){
      exit(0);
    }
    if (z80.PC == 5) {
      if (z80.C == 2) {
        printf("%c\n", z80.E);
      } else if (z80.C == 9) {
        int i = z80.DE;
        while (readMem(i) != 0x24) {
          printf("%c", readMem(i));
          i++;
        }
        printf("\n");
      }
    }
  }
}
