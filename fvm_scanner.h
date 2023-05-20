#pragma once

#include <stddef.h>

typedef struct Span {
  const char* start;
  size_t length;
} Span;

typedef enum TokenType {
  TOK_LABLE,
  TOK_IDENTIFIER,
  TOK_INTLITERAL,
  TOK_CHARLITERAL,
  TOK_EOF,
} TokenType;

typedef struct Token {
  TokenType type;
  Span span;
} Token;

Span span_new(const char* start, size_t length);
Token token_new(TokenType type, Span span);

void scanner_init(const char* input);
Token scanner_get_token();
