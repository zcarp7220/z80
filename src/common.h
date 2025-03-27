#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "json.h"
extern uint8_t json[8704];
extern uint8_t ram[0xFFFF];
extern bool success;
void step(struct json_array_element_s *myTests);
extern inline uint8_t readMem(uint16_t addr);
extern inline void writeMem(uint16_t addr, uint8_t data);
