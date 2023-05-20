#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "fvm_scanner.h"

Span span_new(const char* start, size_t length) {
  Span span;
  span.start = start;
  span.length = length;

  return span;
}

Span span_from(const char* start) {
  Span span;
  span.start = start;
  span.length = 0;

  while (span.start[span.length])
    span.length += 1;

  return span;
}

bool span_equals(Span lhs, Span rhs) {
  if (lhs.length != rhs.length)
    return false;

  for (size_t i = 0; i < lhs.length; i++) {
    if (lhs.start[i] != rhs.start[i])
      return false;
  }

  return true;
}

void span_print(Span span) {
  for (size_t i = 0; i < span.length; i++)
    printf("%c", span.start[i]);
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

  switch (current()) {
  case ',':
    advance();
    return token_new(TOK_COMMA, span_new(start, 1));
  case ';': /* skip comments */
    while (current() && current() != '\n')
      advance();

    skip_ws();
    start = g_input;

    if (!current())
      return token_new(TOK_EOF, span_new(start, 0));
  }

  if (isalpha(current()) || current() == '_') {
    size_t length = 0;

    do {
      advance();
      length += 1;
    } while (current() && (isalnum(current()) || current() == '_'));

    Span span = span_new(start, length);

    if (current() == ':') {
      advance();
      return token_new(TOK_LABLE, span);
    }

    if (span_equals(span, span_from("A"))) {
      return token_new(TOK_REG_A, span);
    } else if (span_equals(span, span_from("B"))) {
      return token_new(TOK_REG_B, span);
    } else if (span_equals(span, span_from("C"))) {
      return token_new(TOK_REG_C, span);
    } else if (span_equals(span, span_from("D"))) {
      return token_new(TOK_REG_D, span);
    } else if (span_equals(span, span_from("E"))) {
      return token_new(TOK_REG_E, span);
    } else if (span_equals(span, span_from("F"))) {
      return token_new(TOK_REG_F, span);
    } else if (span_equals(span, span_from("IP"))) {
      return token_new(TOK_REG_IP, span);
    } else if (span_equals(span, span_from("SP"))) {
      return token_new(TOK_REG_IP, span);
    }

    if (span_equals(span, span_from("push"))) {
      return token_new(TOK_PUSH, span);
    } else if (span_equals(span, span_from("pop"))) {
      return token_new(TOK_POP, span);
    } else if (span_equals(span, span_from("mov"))) {
      return token_new(TOK_MOV, span);
    } else if (span_equals(span, span_from("add"))) {
      return token_new(TOK_ADD, span);
    } else if (span_equals(span, span_from("sub"))) {
      return token_new(TOK_SUB, span);
    } else if (span_equals(span, span_from("mul"))) {
      return token_new(TOK_MUL, span);
    } else if (span_equals(span, span_from("div"))) {
      return token_new(TOK_DIV, span);
    } else if (span_equals(span, span_from("cmp"))) {
      return token_new(TOK_CMP, span);
    } else if (span_equals(span, span_from("jmp"))) {
      return token_new(TOK_JMP, span);
    } else if (span_equals(span, span_from("je"))) {
      return token_new(TOK_JE, span);
    } else if (span_equals(span, span_from("jne"))) {
      return token_new(TOK_JNE, span);
    } else if (span_equals(span, span_from("jg"))) {
      return token_new(TOK_JG, span);
    } else if (span_equals(span, span_from("jl"))) {
      return token_new(TOK_JL, span);
    } else if (span_equals(span, span_from("jge"))) {
      return token_new(TOK_JGE, span);
    } else if (span_equals(span, span_from("jle"))) {
      return token_new(TOK_JLE, span);
    }

    return token_new(TOK_IDENTIFIER, span);
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
