#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "fvm_scanner.h"

Span span_new(const char* start, size_t length) {
  Span span;
  span.start = start;
  span.length = length;

  return span;
}

Token token_new(TokenType type, Span span) {
  Token token;
  token.type = type;
  token.span = span;

  return token;
}

static const char* g_input = NULL;

static char current() {
  return *g_input;
}

static void advance() {
  if (current())
    g_input += 1;
}

static void skip_ws() {
  while (current() && isspace(current()))
    advance();
}

Token scanner_get_token() {
  skip_ws();

  const char* start = g_input;

  if (!current())
    return token_new(TOK_EOF, span_new(start, 0));

  if (isalpha(current()) || current() == '_') {
    size_t length = 0;

    do {
      advance();
      length += 1;
    } while (current() && (isalnum(current()) || current() == '_'));

    if (current() == ':') {
      advance();
      return token_new(TOK_LABLE, span_new(start, length));
    }

    return token_new(TOK_IDENTIFIER, span_new(start, length));
  }

  if (isdigit(current())) {
    size_t length = 0;

    do {
      advance();
      length += 1;
    } while (current() && isdigit(current()));

    return token_new(TOK_INTLITERAL, span_new(start, length));
  }

  if (current() == '\'') {
    advance();
    start = g_input;

    bool escaped = false;

    if (current() == '\\') {
      advance();
      advance();
      escaped = true;
    } else {
      advance();
    }

    if (current() != '\'') {
      fprintf(stderr, "ERROR: unclosed char literal!\n");
      exit(1);
    }

    advance();

    if (escaped) {
      return token_new(TOK_CHARLITERAL, span_new(start, 2));
    } else {
      return token_new(TOK_CHARLITERAL, span_new(start, 1));
    }
  }

  fprintf(stderr, "ERROR: found garbage token: '%c'\n", *start);
  exit(1);
}

void scanner_init(const char* input) {
  g_input = input;
}
