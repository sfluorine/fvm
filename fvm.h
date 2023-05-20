#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "fvm_cpu.h"

typedef struct FVM {
  bool running;
  const int64_t* instructions;
  int64_t registers[REG_SIZE];
  int64_t stack[STACK_SIZE];
} FVM;

void fvm_init(FVM* fvm, const int64_t* instructions);
void fvm_execute(FVM* fvm);
