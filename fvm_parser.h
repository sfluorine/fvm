#pragma once

#include <string.h>

#include "fvm_cpu.h"
#include "cvector.h"

typedef struct ParsedInstruction {
  Instruction instruction;
  int64_t arguments[5];
  size_t arguments_len;
} ParsedInstruction;

cvector_vector_type(int64_t) instructions_codegen(cvector_vector_type(ParsedInstruction) pis);

void parser_init(const char* input);
void parser_deinit();
cvector_vector_type(ParsedInstruction) parser_parse();
