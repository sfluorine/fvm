factorial: ; this program calculates the factorial of 10
  mov A, 1
  mov B, 10

loop:
  mul A, B
  sub B, 1

  cmp B, 0
  jne loop

  halt
