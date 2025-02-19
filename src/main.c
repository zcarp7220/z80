#include "common.h"
uint8_t zexall[8704];
uint8_t ram[0xFFFF];
int main(int argc, char *argv[]) {
  FILE *rom;
  rom = fopen("zexdoc.com", "rb");
  fread(zexall, sizeof(uint8_t), 8704, rom);
  fclose(rom);
  return 0;
}
