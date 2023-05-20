#pragma once

#define STACK_SIZE 6000

typedef enum Register {
  REG_A,
  REG_B,
  REG_C,
  REG_D,
  REG_E,
  REG_F,
  REG_IP,
  REG_SP,
  REG_SIZE,
} Register;

typedef enum Flag {
  ZF,
  GT,
  LT,
} Flag;

typedef enum Instruction {
  INS_HALT,
  INS_PUSH,
  INS_PUSHI,
  INS_POP,
  INS_MOV,
  INS_MOVI,
  INS_ADD,
  INS_ADDI,
  INS_SUB,
  INS_SUBI,
  INS_MUL,
  INS_MULI,
  INS_DIV,
  INS_DIVI,
} Instruction;
