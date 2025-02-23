#include "z80.h"
#include "common.h"
cpu_t z80;

void step(struct json_object_s *inital, struct json_object_s *final, struct json_string_s *name) {
  printf("Loading test: %s\n", name->string);

  runOpcode(&z80);
}
inline int readMem(uint16_t addr) { return ram[addr]; }
inline void writeMem(uint8_t data, uint16_t addr) { ram[addr] = data; }
