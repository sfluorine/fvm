#include <stdio.h>
#include <stdlib.h>

#include "fvm.h"
#include "fvm_parser.h"

int main(int argc, char** argv) {
  if (argc < 2)
    return 1;

  FILE* file = fopen(argv[1], "r");

  if (!file) {
    fprintf(stderr, "ERROR: cannot open: '%s'\n", argv[1]);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buffer = malloc(sizeof(char) * length);

  if (!buffer) {
    fprintf(stderr, "ERROR: cannot allocate memory!\n");
    return 1;
  }

  fread(buffer, length - 1, 1, file);
  buffer[length - 1] = 0;
  fclose(file);

  parser_init(buffer);

  cvector_vector_type(ParsedInstruction) parsed_instructions = parser_parse();

  parser_deinit();
  free(buffer);

  cvector_vector_type(int64_t) instructions = instructions_codegen(parsed_instructions);
  cvector_free(parsed_instructions);

  FVM vm;
  fvm_init(&vm, instructions);
  fvm_execute(&vm);

  cvector_free(instructions);
}
