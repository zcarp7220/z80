#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
extern uint8_t json[8704];
extern uint8_t ram[0xFFFF];

extern inline int readMem(uint16_t addr);
extern inline void writeMem(uint8_t data, uint16_t addr);
