#include "z80.h"
#include "common.h"
cpu_t z80;
void set_value(void *var, const char *value, char type) {
  switch (type) {
  case 'w':
    *(uint16_t *)var = (uint16_t)atoi(value);
    break;
  case 'b':
    *(uint8_t *)var = (uint8_t)atoi(value);
    break;
  case 'f':
    *(bool *)var = atoi(value) != 0;
    break;
  }
}
void step(struct json_object_s *inital, struct json_object_s *final, struct json_string_s *name) {
  printf("Loading: %s| Loading registers| ", name->string);
  struct objects {
    void *reg;
    char width;
  };

  struct objects registers[] = {
      {&z80.PC, 'w'},
      {&z80.SP, 'w'},
      {&z80.A, 'b'},
      {&z80.B, 'b'},
      {&z80.C, 'b'},
      {&z80.D, 'b'},
      {&z80.E, 'b'},
      {&z80.F, 'b'},
      {&z80.H, 'b'},
      {&z80.L, 'b'},
      {&z80.I, 'b'},
      {&z80.R, 'b'},
      {&z80.EI, 'f'},
      {&z80.IX, 'w'},
      {&z80.IY, 'w'},
      {&z80.AFp, 'w'},
      {&z80.BCp, 'w'},
      {&z80.DEp, 'w'},
      {&z80.HLp, 'w'},
      {&z80.IM, 'b'},
      {&z80.IFF1, 'f'},
      {&z80.IFF2, 'f'},
  };
  size_t length = (sizeof(registers) / sizeof(registers[0]));
  struct json_object_element_s *initalObjects = inital->start;
  for (size_t i = 0; i < length + 3; i++) {
    set_value(registers[i].reg, json_value_as_number(initalObjects->value)->number, registers[i].width);
    initalObjects = initalObjects->next;
  }
  printf("Loading RAM| ");
  struct json_array_s *ramArr = json_value_as_array(initalObjects->value);
  size_t i = 0;
  for (struct json_array_element_s *ramElement = ramArr->start; i < ramArr->length; ramElement = ramElement->next) {

    struct json_array_s *ramArray = json_value_as_array(ramElement->value);
    struct json_value_s *firstValue = ramArray->start->value;
    struct json_value_s *secondValue = ramArray->start->next->value;

    int address = atoi(json_value_as_number(firstValue)->number);
    int value = atoi(json_value_as_number(secondValue)->number);

    writeMem(address, value);
    i++;
  }
  printf("Running test: %s:", name->string);
  runOpcode(&z80);
}
inline int readMem(uint16_t addr) { return ram[addr]; }
inline void writeMem(uint16_t addr, uint8_t data) { ram[addr] = data; }
