# FVM (Fluorine Virtual Machine) :computer:

It's an assembler and virtual machine with possibly naive implementation.<br>
I've never built an assembler or virtual machine before so... Bear with me :^)

## Project Background :man_technologist:

I want to make a programming language based on this vm.

## Opcodes :open_book:

Here's a list of the vm's opcodes:

```asm
halt 
push
pop
mov
add
sub
mul
div
cmp
jmp
je
jne
jg
jl
jge
jle
```

## Program Example :memo:

```asm
factorial: ; this program calculates the factorial of 10
  mov A, 1
  mov B, 10

loop:
  mul A, B
  sub B, 1

  cmp B, 0
  jne loop

  halt ; always remember to put this instruction at the end of your program!
```

```bash
# if you have a linux machine and clang installed:
./build.sh

# run an example
./fvm example/factorial
```
