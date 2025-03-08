#include "common.h"
#include "json.h"
uint8_t ram[0xFFFF];
inline void writeMem(uint16_t addr, uint8_t data) { ram[addr] = data; }
inline int readMem(uint16_t addr) { return ram[addr]; }
int main(int argc, char *argv[]) {
  if (argv[1] == NULL) {
    printf("Please Enter A File Name\n");
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("File Does Not Exist");
  }
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *json = (char *)malloc(fileSize * sizeof(char) + 1);
  fread(json, 1, fileSize, file);
  json[fileSize] = '\0';
  fclose(file);
  struct json_value_s *root = json_parse(json, fileSize);
  struct json_array_element_s *myTests = json_value_as_array(root)->start;
  while (myTests->next != NULL) {
    struct json_object_element_s *currentTest = json_value_as_object(myTests->value)->start->next;
    struct json_object_s *initalValue = json_value_as_object(currentTest->value);
    struct json_object_s *expectedFinalValue = json_value_as_object(currentTest->next->value);
    step(initalValue, expectedFinalValue, json_value_as_string(json_value_as_object(myTests->value)->start->value));
    myTests = myTests->next;
  }
  if (success) {
    printf("You Passed the test!");
  }
  free(root);
  return 0;
}
