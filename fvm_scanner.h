#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Span {
  const char* start;
  size_t length;
} Span;

Span span_new(const char* start, size_t length);
Span span_from(const char* start);
bool span_equals(Span lhs, Span rhs);
void span_print(FILE* stream, Span span);

typedef enum TokenType {
  TOK_LABLE,
  TOK_IDENTIFIER,
  TOK_INTLITERAL,
  TOK_CHARLITERAL,

  TOK_COMMA,

  TOK_HALT,
  TOK_PUSH,
  TOK_POP,
  TOK_MOV,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_DIV,
  TOK_CMP,
  TOK_JMP,
  TOK_JE,
  TOK_JNE,
  TOK_JG,   
  TOK_JL,   
  TOK_JGE,  
  TOK_JLE,  

  TOK_REG_A,
  TOK_REG_B,
  TOK_REG_C,
  TOK_REG_D,
  TOK_REG_E,
  TOK_REG_F,
  TOK_REG_IP,
  TOK_REG_SP,

  TOK_EOF,
} TokenType;

typedef struct Token {
  TokenType type;
  Span span;
} Token;

Token token_new(TokenType type, Span span);

void scanner_init(const char* input);
Token scanner_get_token();
