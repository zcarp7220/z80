#include "common.h"
#include <json-c/json.h>
uint8_t zexall[8704];
uint8_t ram[0xFFFF];
int main(int argc, char *argv[]) {
  if (argv[1] == NULL) {
    printf("Please Enter A File Name");
    return 1;
  }
  FILE *rom;
  rom = fopen(argv[1], "rb");
  fread(zexall, sizeof(uint8_t), 8704, rom);
  fclose(rom);
  return 0;
}
