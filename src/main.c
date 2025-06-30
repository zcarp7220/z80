
#include "common.h"
#include "json.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t json[8704];

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Error: Please provide both the mode and the file.\n");
    printf("Usage: %s <mode> <file>\n", argv[0]);
    return 1;
  }
  FILE *file = fopen(argv[2], "r");
  if (file == NULL) {
    printf("Error: File doesn't exist.\n");
    return 1;
  }
  if (strcmp(argv[1], "json") == 0) {
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *json = (char *)malloc(fileSize + 1);
    if (json == NULL) {
      printf("Error: Memory allocation failed.\n");
      fclose(file);
      return 1;
    }
    size_t bytesRead = fread(json, 1, fileSize, file);
    if (bytesRead != fileSize) {
      printf("Error: File read failed.\n");
      free(json);
      fclose(file);
      return 1;
    }
    json[fileSize] = '\0';
    fclose(file);
    struct json_value_s *root = json_parse(json, fileSize);
    if (root == NULL) {
      printf("Error: Failed to parse JSON.\n");
      free(json);
      return 1;
    }
    struct json_array_element_s *myTests = json_value_as_array(root)->start;
    while (myTests != NULL) {
      jsonStep(myTests);
      myTests = myTests->next;
    }

    if (success) {
      // printf("You Passed the test!");
    }

    free(json);
    free(root);
  } else if (strcmp(argv[1], "zex") == 0) {
    fseek(file, 0, SEEK_SET);
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (fileSize > (0xFFFF - 0x100)) {
      printf("Error: File is too large.\n");
      fclose(file);
      return 1;
    }
    uint8_t *buffer = (uint8_t *)malloc(fileSize);
    if (buffer == NULL) {
      printf("Error: Memory allocation failed.\n");
      fclose(file);
      return 1;
    }
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
      printf("Error: File read failed.\n");
      free(buffer);
      fclose(file);
      return 1;
    }
    fclose(file);
    zexInit(buffer, fileSize);
    free(buffer);
    zexStep();
  } else {
    printf("Error: Please specify a valid mode (json or zex).\n");
  }
  return 0;
}
