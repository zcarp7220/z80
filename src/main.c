#include "common.h"
#include "json.h"
uint8_t ram[0xFFFF];
int main(int argc, char *argv[]) {
  if (argv[1] == NULL) {
    printf("Please Enter A File Name");
    return 1;
  }
  FILE *file = fopen(argv[1], "r");
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *json = (char *)malloc(fileSize * sizeof(char) + 1);
  fread(json, 1, fileSize, file);
  json[fileSize] = '\0';
  fclose(file);
  struct json_value_s *root = json_parse(json, fileSize);
  struct json_array_s *myTests = json_value_as_array(root->payload);
  return 0;
}
