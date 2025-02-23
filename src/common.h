#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "json.h"
extern uint8_t json[8704];
extern uint8_t ram[0xFFFF];
void step(struct json_object_s *inital, struct json_object_s *final, struct json_string_s *name);
extern inline int readMem(uint16_t addr);
extern inline void writeMem(uint16_t addr, uint8_t data);
