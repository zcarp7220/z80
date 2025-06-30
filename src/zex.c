#include "common.h"
#include "cpu/z80.h"
uint8_t zexRam[0x10000];
void zexInit(uint8_t *buffer, size_t size) {
  zexRam[0x0000] = 0xED;
  zexRam[0x0001] = 0x41;
  zexRam[0x0005] = 0xDB;
  zexRam[0x0006] = 0x00;
  zexRam[0x0007] = 0xC9;
  for (size_t i = 0; i < size; ++i) {
    zexRam[0x100 + i] = buffer[i];
  }
}
void memWrite(void *unused, uint16_t addr, uint8_t data) {
  zexRam[addr] = data;
}

uint8_t memRead(void *unused, uint16_t addr) {
  return zexRam[addr];
}
void out(cpu_t *z80, uint16_t addr, uint8_t data) {
  exit(0);
}
uint8_t in(cpu_t *z80, uint16_t addr) {
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
  z80.PC = 0x100;
  z80.cycles = 1;
  for (;;) {
    // fprintf(stderr, "PC:%X A:%02X F:%02X  B:%02X C:%02X  D:%02X E:%02X  H:%02X L:%02X P:%02X\n",
    //        z80.PC, z80.A, z80.F,
    //         z80.B, z80.C,
    //         z80.D, z80.E,
    //         z80.H, z80.L, z80.SP);
    if (z80.PC == 0) {
      exit(0);
    }
    cpuStep(&z80);
  }
}
