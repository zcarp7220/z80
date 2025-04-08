#include "common.h"
#include "z80.h"
uint8_t ram[0xFFFF];
void zexInit(uint8_t* buffer, size_t size){
  ram[0x0000] = 0xD3;
  ram[0x0001] = 0x00;
  ram[0x0005] = 0xDB;
  ram[0x0006] = 0x00;
  ram[0x0007] = 0xC9;
  for (size_t i = 0; i < size; ++i) {
    ram[0x100 + i] = buffer[i];
  }
}
void memWrite(void* unused, uint16_t addr ,uint8_t data) {
  fprintf(stderr,"0x%X written to 0x%X\n", data, addr);
  ram[addr] = data;
}

 uint8_t memRead(void* unused,uint16_t addr) {
  return ram[addr];
}
void zexStep() {
  cpu_t z80;
  z80Init(&z80);
  z80.readByte= memRead;
  z80.writeByte = memWrite;
  z80.PC = 0x100;
  z80.cycles = 1;
  for (;;) {   
    fprintf(stderr, "PC: 0x%X, 3 bytes after PC: 0x%X 0x%X 0x%X\n", z80.PC, memRead(NULL, z80.PC), memRead(NULL, z80.PC + 1), memRead(NULL, z80.PC + 2));
    fprintf(stderr, "Cycles: %lld A: 0x%02X  B: 0x%02X C: 0x%02X  D: 0x%02X E: 0x%02X  H: 0x%02X L: 0x%02X SP: 0x%02X\n",
      z80.cycles, z80.A,
      z80.B, z80.C,  
      z80.D, z80.E,  
      z80.H, z80.L, z80.SP);
      cpuStep(&z80);
    if (z80.PC == 5) {
      if (z80.C == 2) {
        printf("%c\n", z80.E);
      } else if (z80.C == 9) {
        int i = z80.DE;
        while (memRead(NULL, i) != 0x24) {
          printf("%c", memRead(NULL, i));
          i++;
        }
        printf("\n");
      }
    }
  }
}
