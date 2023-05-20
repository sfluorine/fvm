#include "fvm.h"
#include "fvm_cpu.h"

int main(void) {
  int64_t instructions[] = {
    INS_PUSHI, 34,
    INS_PUSHI, 35,

    INS_POP, REG_B,
    INS_POP, REG_A,

    INS_ADD, REG_A, REG_B,
    INS_MOVI, REG_B, 0,

    INS_HALT,
  };

  FVM vm;
  fvm_init(&vm, instructions);
  fvm_execute(&vm);
}
