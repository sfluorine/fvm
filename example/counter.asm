counter:
  mov A, 1

loop:
  add A, 1
  cmp A, 10

  jl loop

  halt
