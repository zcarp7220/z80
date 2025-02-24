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
    char name[5];
  };

  struct objects registers[] = {
      {&z80.PC, 'w', "PC"},
      {&z80.SP, 'w', "SP"},
      {&z80.A, 'b', "A"},
      {&z80.B, 'b', "B"},
      {&z80.C, 'b', "C"},
      {&z80.D, 'b', "D"},
      {&z80.E, 'b', "E"},
      {&z80.F, 'b', "F"},
      {&z80.H, 'b', "H"},
      {&z80.L, 'b', "L"},
      {&z80.I, 'b', "I"},
      {&z80.R, 'b', "R"},
      {&z80.EI, 'f', "EI"},
      {&z80.IX, 'w', "IX"},
      {&z80.IY, 'w', "IY"},
      {&z80.AFp, 'w', "AF'"},
      {&z80.BCp, 'w', "BC'"},
      {&z80.DEp, 'w', "DE'"},
      {&z80.HLp, 'w', "HL'"},
      {&z80.IM, 'b', "IM"},
      {&z80.IFF1, 'f', "IFF1"},
      {&z80.IFF2, 'f', "IFF2"},
  };
  size_t length = (sizeof(registers) / sizeof(registers[0]));
  struct json_object_element_s *initalObjects = inital->start;
  for (size_t i = 0; i < length;) {
    if (!(strcmp(initalObjects->name->string, "p") == 0 || strcmp(initalObjects->name->string, "q") == 0 || strcmp(initalObjects->name->string, "wz") == 0)) {
      set_value(registers[i].reg, json_value_as_number(initalObjects->value)->number, registers[i].width);
      i++;
    }
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
  printf("Running test: %s| ", name->string);
  runOpcode(&z80);
  struct json_object_element_s *finalObjects = final->start;
  int actual = 0;
  struct objects finalRegisters[] = {
      {&z80.A, 'b', "A"},
      {&z80.B, 'b', "B"},
      {&z80.C, 'b', "C"},
      {&z80.D, 'b', "D"},
      {&z80.E, 'b', "E"},
      {&z80.F, 'b', "F"},
      {&z80.H, 'b', "H"},
      {&z80.L, 'b', "L"},
      {&z80.I, 'b', "I"},
      {&z80.R, 'b', "R"},
      {&z80.AFp, 'w', "AF'"},
      {&z80.BCp, 'w', "BC'"},
      {&z80.DEp, 'w', "DE'"},
      {&z80.HLp, 'w', "HL'"},
      {&z80.IX, 'w', "IX"},
      {&z80.IY, 'w', "IY"},
      {&z80.PC, 'w', "PC"},
      {&z80.SP, 'w', "SP"},
      {&z80.IFF1, 'f', "IFF1"},
      {&z80.IFF2, 'f', "IFF2"},
      {&z80.IM, 'b', "IM"},
      {&z80.EI, 'f', "EI"},

  };
  for (size_t j = 0; j < length;) {
    if (!(strcmp(finalObjects->name->string, "p") == 0 || strcmp(finalObjects->name->string, "q") == 0 || strcmp(finalObjects->name->string, "wz") == 0)) {
      if (finalRegisters[j].width == 'b') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'w') {
        actual = *(unsigned short *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'f') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      }
      if ((actual != atoi(json_value_as_number(finalObjects->value)->number))) {
        printf("Fail: Expected value for %s is 0x%X, Actual value is %s at 0x%X\n", finalObjects->name->string, atoi(json_value_as_number(finalObjects->value)->number), finalRegisters[j].name, actual);
        return;
      }
      j++;
    }
    finalObjects = finalObjects->next;
  }
  printf("Pass\n");
}
inline int readMem(uint16_t addr) { return ram[addr]; }
inline void writeMem(uint16_t addr, uint8_t data) { ram[addr] = data; }
