#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
extern uint8_t zexall[8704];
extern uint8_t ram[0xFFFF];

extern inline int readMem(uint16_t addr);
extern inline void writeMem(uint8_t data, uint16_t addr);
