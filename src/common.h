#include "json.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
extern uint8_t json[8704];
extern uint8_t ram[0xFFFF];
extern bool success;
void jsonStep(struct json_array_element_s *myTests);
void cpuStep();
void zexStep();
extern uint8_t readMem(uint16_t addr);
extern void writeMem(uint16_t addr, uint8_t data);
