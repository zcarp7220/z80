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
  rewind(file);
  char *json = malloc(fileSize * sizeof(char) + 1);
  json[fileSize] = '\0';
  fclose(file);
  struct json_value_s *root = json_parse(json, strlen(json));
  struct json_object_s *object = json_value_as_object(root);
  return 0;
}
