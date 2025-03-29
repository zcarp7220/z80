#include "common.h"
#include "json.h"
uint8_t ram[0xFFFF];
inline void writeMem(uint16_t addr, uint8_t data) { ram[addr] = data; }
inline uint8_t readMem(uint16_t addr) { return ram[addr]; }
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Error: No file provided\n");
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Error: File doesnt Exist\n");
    return 1;
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
    step(myTests);
    myTests = myTests->next;
  }
  if (success) {
    printf("You Passed the test!");
  }
  free(root);
  return 0;
}
