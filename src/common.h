#include "json.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
extern bool success;
void jsonStep(struct json_array_element_s *myTests);
void zexStep();
void zexInit(uint8_t *buffer, size_t size);
