#include "../z80.h"
#include "common.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                                       \
  ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'), ((byte) & 0x20 ? '1' : '0'),           \
      ((byte) & 0x10 ? '1' : '0'), ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),       \
      ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

struct json_object_element_s *currentTest = NULL;
struct json_object_s *initalValue = NULL;
struct json_object_s *expectedFinalValue = NULL;
struct json_string_s *name = NULL;
int address = 0;
int data = 0;
cpu_t z80 = {0};
bool success = true;
uint8_t ram[0xFFFF];
struct json_array_s *expectedPorts = NULL;

void writeMem(cpu_t *unused, uint16_t addr, uint8_t data) { ram[addr] = data; }

uint8_t readMem(cpu_t *unused, uint16_t addr) { return ram[addr]; }

uint8_t input(cpu_t *unused, uint16_t addr) {
  if (atoi(json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->value)
              ->number) == addr) {
    return atoi(
        json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->next->value)
            ->number);
  }
  printf("Failed, The expected address is 0x%X but got 0x%X instead \n", addr,
      atoi(json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->value)
              ->number));
  return 0xEA;
}
void output(cpu_t *unused, uint8_t addr, uint8_t data) {
  if (atoi(json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->value)
              ->number) != addr) {
    printf("Addr(0x%X) doesnt line up with expected(0x%X) value\n", addr,
        atoi(json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->value)
                ->number));
  }
  if (atoi(
          json_value_as_number(json_value_as_array(expectedPorts->start->value)->start->next->value)
              ->number) != data) {
    printf("Data(0x%X) doesnt line up with expected(0x%X) value \n", data,
        atoi(json_value_as_number(
            json_value_as_array(expectedPorts->start->value)->start->next->value)
                ->number));
  }
  exit(0);
  return;
}
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
void jsonStep(struct json_array_element_s *myTests) {
  z80Init(&z80);
  z80.readByte = readMem;
  z80.writeByte = writeMem;
  z80.in = input;
  z80.out = output;
  struct json_object_element_s *currentTest = json_value_as_object(myTests->value)->start->next;
  struct json_object_s *initalValue = json_value_as_object(currentTest->value);
  struct json_object_s *expectedFinalValue = json_value_as_object(currentTest->next->value);
  struct json_array_s *cycles = json_value_as_array(currentTest->next->next->value);
  struct json_string_s *name =
      json_value_as_string(json_value_as_object(myTests->value)->start->value);
  if (currentTest->next->next->next != NULL) {
    expectedPorts = json_value_as_array(currentTest->next->next->next->value);
  }
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
  struct json_object_element_s *initalObjects = initalValue->start;
  for (size_t i = 0; i < length;) {
    if (!(strcmp(initalObjects->name->string, "p") == 0 ||
            strcmp(initalObjects->name->string, "q") == 0 ||
            strcmp(initalObjects->name->string, "wz") == 0 ||
            strcmp(initalObjects->name->string, "ei") == 0)) {
      set_value(
          registers[i].reg, json_value_as_number(initalObjects->value)->number, registers[i].width);
      i++;
    }
    initalObjects = initalObjects->next;
  }
  struct json_array_s *ramArr = json_value_as_array(initalObjects->value);
  size_t i = 0;
  for (struct json_array_element_s *ramElement = ramArr->start; i < ramArr->length;
      ramElement = ramElement->next) {

    struct json_array_s *ramArray = json_value_as_array(ramElement->value);
    struct json_value_s *firstValue = ramArray->start->value;
    struct json_value_s *secondValue = ramArray->start->next->value;

    int address = atoi(json_value_as_number(firstValue)->number);
    int value = atoi(json_value_as_number(secondValue)->number);
    // printf("Ram address 0x%X and value is 0x%X\n", address, value);
    writeMem(NULL, address, value);
    i++;
  }

  // STEP
  cpuStep(&z80);
  struct json_object_element_s *finalObjects = expectedFinalValue->start;
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

  };
  for (size_t j = 0; j < length;) {
    if (!(strcmp(finalObjects->name->string, "p") == 0 ||
            strcmp(finalObjects->name->string, "q") == 0 ||
            strcmp(finalObjects->name->string, "wz") == 0 ||
            strcmp(initalObjects->name->string, "ei") == 0)) {
      if (finalRegisters[j].width == 'b') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'w') {
        actual = *(unsigned short *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'f') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      }
      int expected = atoi(json_value_as_number(finalObjects->value)->number);
      if (actual != expected) {
        if (strcmp(finalRegisters[j].name, "F") != 0) {
          printf("Fail: Expected value for %s is 0x%X, Actual value for %s is 0x%X on test %s\n",
              finalObjects->name->string, atoi(json_value_as_number(finalObjects->value)->number),
              finalRegisters[j].name, actual, name->string);
        } else {
          if ((actual & 0xD7) == (expected & 0xD7)) {
            finalObjects = finalObjects->next;
            j++;
            continue;
          }
          printf("Fail: Expected value for %s is " BYTE_TO_BINARY_PATTERN
                 ", Actual value for %s is " BYTE_TO_BINARY_PATTERN
                 " Difference is " BYTE_TO_BINARY_PATTERN " on test %s\n",
              finalObjects->name->string,
              BYTE_TO_BINARY(atoi(json_value_as_number(finalObjects->value)->number)),
              finalRegisters[j].name, BYTE_TO_BINARY(actual),
              BYTE_TO_BINARY(actual ^ (atoi(json_value_as_number(finalObjects->value)->number))),
              name->string);
          printf("                                                                                 "
                 "   ^^^^^^^^\n");
          printf("                                                                                 "
                 "   SZ5H3PNC\n");
        }
        success = false;
      }
      j++;
    }
    finalObjects = finalObjects->next;
  }
  size_t ramCheckIndex = 0;
  struct json_array_s *finalRamArr = json_value_as_array(finalObjects->next->next->next->value);
  for (struct json_array_element_s *ramElement = finalRamArr->start;
      ramCheckIndex < finalRamArr->length; ramElement = ramElement->next) {
    struct json_array_s *ramArray = json_value_as_array(ramElement->value);
    struct json_value_s *firstValue = ramArray->start->value;
    struct json_value_s *secondValue = ramArray->start->next->value;

    int address = atoi(json_value_as_number(firstValue)->number);
    int expectedValue = atoi(json_value_as_number(secondValue)->number);
    int actualValue = readMem(NULL, address);

    if (expectedValue != actualValue) {
      printf("RAM Test Fail: Expected value at 0x%X is 0x%X, Actual value is 0x%X on test %s\n",
          address, expectedValue, actualValue, name->string);
      success = false;
    }
    ram[address] = 0;
    ramCheckIndex++;
  }
  for (int i = 0; i < 0x1000; i++) {
    if (ram[i] != 0) {
      printf("Unexpected Value in RAM! At Address %X I found %X which should be a 0\n", i, ram[i]);
      success = false;
    }
  }

  long cyclesLength = cycles->length;
  if (cyclesLength != z80.cycles) {
    printf("Cycle Mismatch, Expected %ld cycles, instead got %ld cycles on test %s| Diff: %ld\n",
        cyclesLength, z80.cycles, name->string, (cyclesLength - z80.cycles));
    success = false;
  }
  if (!success) {
    exit(0);
  }
}
