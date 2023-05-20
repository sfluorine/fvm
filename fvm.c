#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fvm.h"

static int64_t fetch(FVM* vm, int offset) {
  return vm->instructions[vm->registers[REG_IP] - offset];
}

static void advance(FVM* vm) {
  vm->registers[REG_IP] += 1;
}

static void push(FVM* vm, int64_t value) {
  vm->registers[REG_SP] += 1;
  vm->stack[vm->registers[REG_SP]] = value;
}

static void pop(FVM* vm) {
  vm->registers[REG_SP] -= 1;
}

static void debug(FVM* vm) {
  printf("REGISTERS: ");

  for (int i = 0; i < REG_SIZE; i++)
    printf("[%ld] ", vm->registers[i]);

  printf("\n");
}

static void eval(FVM* vm) {
  switch (fetch(vm, 0)) {
  case INS_HALT:
    vm->running = false;
    break;
  case INS_PUSH:
    advance(vm);
    push(vm, vm->registers[fetch(vm, 0)]);
    advance(vm);
    break;
  case INS_PUSHI:
    advance(vm);
    push(vm, fetch(vm, 0));
    advance(vm);
    break;
  case INS_POP:
    advance(vm);
    vm->registers[fetch(vm, 0)] = vm->stack[vm->registers[REG_SP]];
    advance(vm);
    pop(vm);
    break;
  case INS_MOV:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_MOVI:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = fetch(vm, 0);
    advance(vm);
    break;
  case INS_ADD:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] + vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_ADDI:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] + fetch(vm, 0);
    advance(vm);
    break;
  case INS_SUB:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] - vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_SUBI:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] - fetch(vm, 0);
    advance(vm);
    break;
  case INS_MUL:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] * vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_MULI:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] * fetch(vm, 0);
    advance(vm);
    break;
  case INS_DIV:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] / vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_DIVI:
    advance(vm);
    advance(vm);
    vm->registers[fetch(vm, 1)] = vm->registers[fetch(vm, 1)] / fetch(vm, 0);
    advance(vm);
    break;
  case INS_CMP:
    advance(vm);
    advance(vm);
    vm->flags[FLAG_EQ] = vm->registers[fetch(vm, 1)] == vm->registers[fetch(vm, 0)];
    vm->flags[FLAG_GT] = vm->registers[fetch(vm, 1)]  > vm->registers[fetch(vm, 0)];
    vm->flags[FLAG_LT] = vm->registers[fetch(vm, 1)]  < vm->registers[fetch(vm, 0)];
    advance(vm);
    break;
  case INS_CMPI:
    advance(vm);
    advance(vm);
    vm->flags[FLAG_EQ] = vm->registers[fetch(vm, 1)] == fetch(vm, 0);
    vm->flags[FLAG_GT] = vm->registers[fetch(vm, 1)]  > fetch(vm, 0);
    vm->flags[FLAG_LT] = vm->registers[fetch(vm, 1)]  < fetch(vm, 0);
    advance(vm);
    break;
  case INS_JMP:
    advance(vm);
    vm->registers[REG_IP] = fetch(vm, 0);
    break;
  case INS_JE:
    advance(vm);

    if (vm->flags[FLAG_EQ]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  case INS_JNE:
    advance(vm);

    if (!vm->flags[FLAG_EQ]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  case INS_JG:
    advance(vm);

    if (vm->flags[FLAG_GT]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  case INS_JL:
    advance(vm);

    if (vm->flags[FLAG_LT]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  case INS_JGE:
    advance(vm);

    if (vm->flags[FLAG_GT] || vm->flags[FLAG_EQ]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  case INS_JLE:
    advance(vm);

    if (vm->flags[FLAG_LT] || vm->flags[FLAG_EQ]) {
      vm->registers[REG_IP] = fetch(vm, 0);
    } else {
      advance(vm);
    }
    break;
  default:
    fprintf(stderr, "ERROR: unknown instruction: %ld\n", fetch(vm, 0));
    exit(1);
  }
}

void fvm_execute(FVM* vm) {
  while (vm->running) {
    eval(vm);
    debug(vm);
  }
}

void fvm_init(FVM* vm, const int64_t* instructions) {
  if (instructions)
    vm->running = true;

  vm->instructions = instructions;

  for (int i = 0; i < REG_SIZE; i++)
    vm->registers[i] = 0;

  vm->registers[REG_SP] = -1;
}
