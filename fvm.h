#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "fvm_cpu.h"

typedef struct FVM {
  bool running;
  const int64_t* instructions;
  int64_t registers[REG_SIZE];
  int64_t flags[FLAG_SIZE];
  int64_t stack[STACK_SIZE];
} FVM;

void fvm_execute(FVM* vm);
void fvm_init(FVM* vm, const int64_t* instructions);
