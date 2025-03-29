#include "z80.h"
#include "common.h"

cpu_t z80 = {0};
bool success = true;
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
void step(struct json_array_element_s *myTests) {
  struct json_object_element_s *currentTest = json_value_as_object(myTests->value)->start->next;
  struct json_object_s *initalValue = json_value_as_object(currentTest->value);
  struct json_object_s *expectedFinalValue = json_value_as_object(currentTest->next->value);
  struct json_string_s *name = json_value_as_string(json_value_as_object(myTests->value)->start->value);
  struct json_array_s *expectedPorts = NULL;
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
    if (!(strcmp(initalObjects->name->string, "p") == 0 || strcmp(initalObjects->name->string, "q") == 0 || strcmp(initalObjects->name->string, "wz") == 0 || strcmp(initalObjects->name->string, "ei") == 0)) {
      set_value(registers[i].reg, json_value_as_number(initalObjects->value)->number, registers[i].width);
      i++;
    }
    initalObjects = initalObjects->next;
  }
  struct json_array_s *ramArr = json_value_as_array(initalObjects->value);
  size_t i = 0;
  for (struct json_array_element_s *ramElement = ramArr->start; i < ramArr->length; ramElement = ramElement->next) {

    struct json_array_s *ramArray = json_value_as_array(ramElement->value);
    struct json_value_s *firstValue = ramArray->start->value;
    struct json_value_s *secondValue = ramArray->start->next->value;

    int address = atoi(json_value_as_number(firstValue)->number);
    int value = atoi(json_value_as_number(secondValue)->number);
    // printf("Ram address 0x%X and value is 0x%X\n", address, value);
    writeMem(address, value);
    i++;
  }

  // Da Meat
  handleInterupts();
  if (!z80.halt) {
    runOpcode(readMem(z80.PC));
  } else {
    runOpcode(0x0);
  }

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
    if (!(strcmp(finalObjects->name->string, "p") == 0 || strcmp(finalObjects->name->string, "q") == 0 || strcmp(finalObjects->name->string, "wz") == 0 || strcmp(initalObjects->name->string, "ei") == 0)) {
      if (finalRegisters[j].width == 'b') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'w') {
        actual = *(unsigned short *)finalRegisters[j].reg;
      } else if (finalRegisters[j].width == 'f') {
        actual = *(unsigned char *)finalRegisters[j].reg;
      }
      if ((actual != atoi(json_value_as_number(finalObjects->value)->number))) {
        if (strcmp(finalRegisters[j].name, "F") != 0) {
          printf("Fail: Expected value for %s is 0x%X, Actual value for %s is 0x%X on test %s\n", finalObjects->name->string, atoi(json_value_as_number(finalObjects->value)->number), finalRegisters[j].name, actual, name->string);
        } else {
          printf("Fail: Expected value for %s is " BYTE_TO_BINARY_PATTERN ", Actual value for %s is " BYTE_TO_BINARY_PATTERN " Diffrence is " BYTE_TO_BINARY_PATTERN " on test %s\n", finalObjects->name->string, BYTE_TO_BINARY(atoi(json_value_as_number(finalObjects->value)->number)), finalRegisters[j].name, BYTE_TO_BINARY(actual), BYTE_TO_BINARY(actual ^ (atoi(json_value_as_number(finalObjects->value)->number))), name->string);
          printf("                                                                                    ^^^^^^^^\n");
          printf("                                                                                    SZ5H3PNC\n");
        }
        success = false;
        exit(0);
      }
      j++;
    }
    finalObjects = finalObjects->next;
  }

  size_t ramCheckIndex = 0;
  struct json_array_s *finalRamArr = json_value_as_array(finalObjects->next->next->next->value);
  for (struct json_array_element_s *ramElement = finalRamArr->start; ramCheckIndex < finalRamArr->length; ramElement = ramElement->next) {
    struct json_array_s *ramArray = json_value_as_array(ramElement->value);
    struct json_value_s *firstValue = ramArray->start->value;
    struct json_value_s *secondValue = ramArray->start->next->value;

    int address = atoi(json_value_as_number(firstValue)->number);
    int expectedValue = atoi(json_value_as_number(secondValue)->number);
    int actualValue = readMem(address);

    if (expectedValue != actualValue) {
      printf("RAM Test Fail: Expected value at 0x%X is 0x%X, Actual value is 0x%X on test %s\n", address, expectedValue, actualValue, name->string);
      success = false;
      exit(0);
    }
    ramCheckIndex++;
  }
  if (expectedPorts == NULL) {
    return;
  }

  struct json_array_s *finalPortValues = json_value_as_array(expectedPorts->start->value);
  int address = atoi(json_value_as_number(finalPortValues->start->value)->number);
  int data = atoi(json_value_as_number(finalPortValues->start->next->value)->number);
  char readWrite[10];
  strncpy(readWrite, json_value_as_string(finalPortValues->start->next->next->value)->string, sizeof(readWrite) - 1)[sizeof(readWrite) - 1] = '\0';
  if (!strcmp(readWrite, "w") && !z80.IOwrite) {
    printf("Fail: Expected value for the I/O write is %s, Actual value for I/O write is  %s on test %s\n", readWrite, z80.IOwrite ? "Write" : "NIL", name->string);
    exit(0);
  }
  if (!strcmp(readWrite, "b") && !z80.IOread) {
    printf("Fail: Expected value for the I/O read is %s, Actual value for I/O read is %s on test %s\n", readWrite, z80.IOread ? "Write" : "NIL", name->string);
    exit(0);
  }
  if (address != z80.address) {
    printf("Fail: Expected value for the address is 0x%X, Actual value for address is 0x%X on test %s\n", address, z80.address, name->string);
    exit(0);
  }
  if (data != z80.data) {
    printf("Fail: Expected value for the data is 0x%X, Actual value for data is 0x%X on test %s\n", data, z80.data, name->string);
    exit(0);
  }
}
void handleInterupts() {
  if (z80.halt) {
  }
}
