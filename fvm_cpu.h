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
  FLAG_EQ,
  FLAG_GT,
  FLAG_LT,
  FLAG_SIZE,
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
  INS_CMP,
  INS_CMPI,
  INS_JMP,
  INS_JE,   
  INS_JNE,  
  INS_JG,   
  INS_JL,   
  INS_JGE,  
  INS_JLE,  
} Instruction;
