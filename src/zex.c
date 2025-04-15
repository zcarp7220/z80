#include "common.h"
#include "z80.h"
uint8_t zexRam[0x10000];
void zexInit(uint8_t *buffer, size_t size) {
  zexRam[0x0000] = 0xD3;
  zexRam[0x0001] = 0x00;
  zexRam[0x0005] = 0xDB;
  zexRam[0x0006] = 0x00;
  zexRam[0x0007] = 0xC9;
  for (size_t i = 0; i < size; ++i) {
    zexRam[0x100 + i] = buffer[i];
  }
}
void memWrite(void *unused, uint16_t addr, uint8_t data) {
  // fprintf(stderr, "0x%X written to 0x%X\n", data, addr);
  zexRam[addr] = data;
}

uint8_t memRead(void *unused, uint16_t addr) {
  return zexRam[addr];
}
void out(cpu_t *z80, uint16_t addr, uint8_t data) {
    exit(0);
}
uint8_t in(cpu_t *z80, uint16_t addr) {
  int varible = addr + z80->A;
  if (z80->C == 2) {
    printf("%c", z80->E);
  } else if (z80->C == 9) {
    int i = z80->DE;
    while (memRead(NULL, i) != 0x24) {
      printf("%c", memRead(NULL, i));
      i++;
    }
  }
  return 0xFF;
}
void zexStep() {
  cpu_t z80;
  z80Init(&z80);
  z80.readByte = memRead;
  z80.writeByte = memWrite;
  z80.out = out;
  z80.in = in;
  z80.PC = 0x99;
  z80.cycles = 1;
  for (;;) {
    //  if (z80.cycles > 5680749720) {
    //    fprintf(stderr, "PC: 0x%X, 3 bytes after PC: 0x%X 0x%X 0x%X\n", z80.PC, memRead(NULL, z80.PC), memRead(NULL, z80.PC + 1), memRead(NULL, z80.PC + 2));
    //     fprintf(stderr, "Cycles: %lld A: 0x%02X  B: 0x%02X C: 0x%02X  D: 0x%02X E: 0x%02X  H: 0x%02X L: 0x%02X SP: 0x%02X\n",
    //             z80.cycles, z80.A,
    //             z80.B, z80.C,
    //             z80.D, z80.E,
    //             z80.H, z80.L, z80.SP);
    //   }

    cpuStep(&z80);
  }
}
