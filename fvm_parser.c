#include <stdio.h>
#include <stdlib.h>

#include "fvm_parser.h"
#include "fvm_scanner.h"

cvector_vector_type(int64_t) instructions_codegen(cvector_vector_type(ParsedInstruction) pis) {
  cvector_vector_type(int64_t) instructions = NULL;

  for (ParsedInstruction* it = cvector_begin(pis); it != cvector_end(pis); ++it) {
    cvector_push_back(instructions, it->instruction);

    for (size_t i = 0; i < it->arguments_len; i++)
      cvector_push_back(instructions, it->arguments[i]);
  }

  return instructions;
}

typedef struct Symbol {
  Span span;
  int64_t address;
} Symbol;

static Symbol symbol_new(Span span, int64_t address) {
  Symbol symbol;
  symbol.span = span;
  symbol.address = address;

  return symbol;
}

static bool is_register(TokenType type) {
  switch (type) {
  case TOK_REG_A:
  case TOK_REG_B:
  case TOK_REG_C:
  case TOK_REG_D:
  case TOK_REG_E:
  case TOK_REG_F:
  case TOK_REG_IP:
  case TOK_REG_SP:
    return true;
  }

  return false;
}

static int64_t from_register(TokenType type) {
  switch (type) {
  case TOK_REG_A:
    return 0;
  case TOK_REG_B:
    return 1;
  case TOK_REG_C:
    return 2;
  case TOK_REG_D:
    return 3;
  case TOK_REG_E:
    return 4;
  case TOK_REG_F:
    return 5;
  case TOK_REG_IP:
    return 6;
  case TOK_REG_SP:
    return 7;
  default:
    fprintf(stderr, "ERROR: trying to convert non register!\n");
    exit(1);
  }
}

static bool is_immediate(TokenType type) {
  switch (type) {
  case TOK_INTLITERAL:
  case TOK_CHARLITERAL:
    return true;
  }

  return false;
}

static int64_t parse_immediate(Token token) {
  switch (token.type) {
  case TOK_INTLITERAL:
    return strtol(token.span.start, NULL, 10);
  case TOK_CHARLITERAL:
    if (token.span.length > 1) {
      switch (*(token.span.start + 1)) {
      case 't':
        return 9;
        break;
      case 'n':
        return 10;
        break;
      case '\'':
        return (int64_t)'\'';
        break;
      case '\\':
        return (int64_t)'\\';
        break;
      default:
        fprintf(stderr, "ERROR: unknown escape character: ");
        span_print(stderr, token.span);
        fprintf(stderr, "\n");
        exit(1);
      }
    }

    return (int64_t)(*token.span.start);
  default:
    fprintf(stderr, "ERROR: trying to parse non immediate!\n");
    exit(1);
  }
}

static size_t g_symtable_len;
static size_t g_symtable_cap;
static Symbol* g_symtable;

static Token g_current;
static int64_t g_address;

static void symtable_insert(Symbol symbol) {
  if (g_symtable_len >= g_symtable_cap) {
    g_symtable_cap += 10;
    g_symtable = realloc(g_symtable, sizeof(Symbol) * g_symtable_cap);

    if (!g_symtable) {
      fprintf(stderr, "ERROR: failed to allocate memory!\n");
      exit(1);
    }
  }

  g_symtable[g_symtable_len] = symbol;
  g_symtable_len += 1;
}

static int symtable_find(Span span) {
  for (size_t i = 0; i < g_symtable_len; i++) {
    if (span_equals(g_symtable[i].span, span))
      return (int)i;
  }

  return -1;
}

static void symtable_print() {
  printf("===============================\n");
  for (size_t i = 0; i < g_symtable_len; i++) {
    span_print(stdout, g_symtable[i].span);
    printf(": %ld\n", g_symtable[i].address);
  }
  printf("===============================\n");
}

static bool expect(TokenType type) {
  return g_current.type == type;
}

static void advance(bool is_advance_addr) {
  if (expect(TOK_EOF))
    return;

  g_current = scanner_get_token();

  if (is_advance_addr)
    g_address += 1;
}

static void match(TokenType type) {
  if (!expect(type)) {
    fprintf(stderr, "ERROR: syntax error!\n");
    exit(1);
  }
}

cvector_vector_type(ParsedInstruction) parser_parse() {
  cvector_vector_type(ParsedInstruction) instructions = NULL;

  while (!expect(TOK_EOF)) {
    if (expect(TOK_LABLE)) {
      symtable_insert(symbol_new(g_current.span, g_address));
      advance(false);
      continue;
    }

    if (expect(TOK_HALT)) {
      advance(true);

      ParsedInstruction halt;
      halt.instruction = INS_HALT;
      halt.arguments_len = 0;
      cvector_push_back(instructions, halt);

      continue;
    }

    if (expect(TOK_PUSH)) {
      advance(true);

      if (is_immediate(g_current.type)) {
        ParsedInstruction pushi;
        pushi.instruction = INS_PUSHI;
        pushi.arguments[0] = parse_immediate(g_current);
        pushi.arguments_len = 1;
        cvector_push_back(instructions, pushi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction push;
      push.instruction = INS_PUSH;
      push.arguments[0] = from_register(g_current.type);
      push.arguments_len = 1;
      cvector_push_back(instructions, push);

      advance(true);
      continue;
    }

    if (expect(TOK_POP)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction pop;
      pop.instruction = INS_PUSH;
      pop.arguments[0] = from_register(g_current.type);
      pop.arguments_len = 1;
      cvector_push_back(instructions, pop);

      advance(true);
      continue;
    }

    if (expect(TOK_MOV)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction movi;
        movi.instruction = INS_MOVI;
        movi.arguments[0] = reg_a;
        movi.arguments[1] = parse_immediate(g_current);
        movi.arguments_len = 2;
        cvector_push_back(instructions, movi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction mov;
      mov.instruction = INS_MOV;
      mov.arguments[0] = reg_a;
      mov.arguments[1] = from_register(g_current.type);
      mov.arguments_len = 2;
      cvector_push_back(instructions, mov);

      advance(true);
      continue;
    }

    if (expect(TOK_ADD)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction addi;
        addi.instruction = INS_ADDI;
        addi.arguments[0] = reg_a;
        addi.arguments[1] = parse_immediate(g_current);
        addi.arguments_len = 2;
        cvector_push_back(instructions, addi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction add;
      add.instruction = INS_ADD;
      add.arguments[0] = reg_a;
      add.arguments[1] = from_register(g_current.type);
      add.arguments_len = 2;
      cvector_push_back(instructions, add);

      advance(true);
      continue;
    }

    if (expect(TOK_SUB)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction subi;
        subi.instruction = INS_SUBI;
        subi.arguments[0] = reg_a;
        subi.arguments[1] = parse_immediate(g_current);
        subi.arguments_len = 2;
        cvector_push_back(instructions, subi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction sub;
      sub.instruction = INS_SUB;
      sub.arguments[0] = reg_a;
      sub.arguments[1] = from_register(g_current.type);
      sub.arguments_len = 2;
      cvector_push_back(instructions, sub);

      advance(true);
      continue;
    }

    if (expect(TOK_MUL)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction muli;
        muli.instruction = INS_MULI;
        muli.arguments[0] = reg_a;
        muli.arguments[1] = parse_immediate(g_current);
        muli.arguments_len = 2;
        cvector_push_back(instructions, muli);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction mul;
      mul.instruction = INS_MUL;
      mul.arguments[0] = reg_a;
      mul.arguments[1] = from_register(g_current.type);
      mul.arguments_len = 2;
      cvector_push_back(instructions, mul);

      advance(true);
      continue;
    }

    if (expect(TOK_DIV)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction divi;
        divi.instruction = INS_DIVI;
        divi.arguments[0] = reg_a;
        divi.arguments[1] = parse_immediate(g_current);
        divi.arguments_len = 2;
        cvector_push_back(instructions, divi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction div;
      div.instruction = INS_DIV;
      div.arguments[0] = reg_a;
      div.arguments[1] = from_register(g_current.type);
      div.arguments_len = 2;
      cvector_push_back(instructions, div);

      advance(true);
      continue;
    }

    if (expect(TOK_CMP)) {
      advance(true);

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      int64_t reg_a = from_register(g_current.type);
      advance(true);

      match(TOK_COMMA);
      advance(false);

      if (is_immediate(g_current.type)) {
        ParsedInstruction cmpi;
        cmpi.instruction = INS_CMPI;
        cmpi.arguments[0] = reg_a;
        cmpi.arguments[1] = parse_immediate(g_current);
        cmpi.arguments_len = 2;
        cvector_push_back(instructions, cmpi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction cmp;
      cmp.instruction = INS_CMP;
      cmp.arguments[0] = reg_a;
      cmp.arguments[1] = from_register(g_current.type);
      cmp.arguments_len = 2;
      cvector_push_back(instructions, cmp);

      advance(true);
      continue;
    }

    if (expect(TOK_JMP)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jmpi;
        jmpi.instruction = INS_JMPI;
        jmpi.arguments[0] = g_symtable[index].address;
        jmpi.arguments_len = 1;
        cvector_push_back(instructions, jmpi);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jmpi;
        jmpi.instruction = INS_JMPI;
        jmpi.arguments[0] = parse_immediate(g_current);
        jmpi.arguments_len = 1;
        cvector_push_back(instructions, jmpi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jmp;
      jmp.instruction = INS_JMP;
      jmp.arguments[0] = from_register(g_current.type);
      jmp.arguments_len = 1;
      cvector_push_back(instructions, jmp);

      advance(true);
      continue;
    }

    if (expect(TOK_JE)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jei;
        jei.instruction = INS_JEI;
        jei.arguments[0] = g_symtable[index].address;
        jei.arguments_len = 1;
        cvector_push_back(instructions, jei);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jei;
        jei.instruction = INS_JEI;
        jei.arguments[0] = parse_immediate(g_current);
        jei.arguments_len = 1;
        cvector_push_back(instructions, jei);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction je;
      je.instruction = INS_JE;
      je.arguments[0] = from_register(g_current.type);
      je.arguments_len = 1;
      cvector_push_back(instructions, je);

      advance(true);
      continue;
    }

    if (expect(TOK_JNE)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jnei;
        jnei.instruction = INS_JNEI;
        jnei.arguments[0] = g_symtable[index].address;
        jnei.arguments_len = 1;
        cvector_push_back(instructions, jnei);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jnei;
        jnei.instruction = INS_JNEI;
        jnei.arguments[0] = parse_immediate(g_current);
        jnei.arguments_len = 1;
        cvector_push_back(instructions, jnei);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jne;
      jne.instruction = INS_JNE;
      jne.arguments[0] = from_register(g_current.type);
      jne.arguments_len = 1;
      cvector_push_back(instructions, jne);

      advance(true);
      continue;
    }

    if (expect(TOK_JG)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jgi;
        jgi.instruction = INS_JGI;
        jgi.arguments[0] = g_symtable[index].address;
        jgi.arguments_len = 1;
        cvector_push_back(instructions, jgi);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jgi;
        jgi.instruction = INS_JGI;
        jgi.arguments[0] = parse_immediate(g_current);
        jgi.arguments_len = 1;
        cvector_push_back(instructions, jgi);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jg;
      jg.instruction = INS_JG;
      jg.arguments[0] = from_register(g_current.type);
      jg.arguments_len = 1;
      cvector_push_back(instructions, jg);

      advance(true);
      continue;
    }

    if (expect(TOK_JL)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jli;
        jli.instruction = INS_JLI;
        jli.arguments[0] = g_symtable[index].address;
        jli.arguments_len = 1;
        cvector_push_back(instructions, jli);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jli;
        jli.instruction = INS_JLI;
        jli.arguments[0] = parse_immediate(g_current);
        jli.arguments_len = 1;
        cvector_push_back(instructions, jli);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jl;
      jl.instruction = INS_JL;
      jl.arguments[0] = from_register(g_current.type);
      jl.arguments_len = 1;
      cvector_push_back(instructions, jl);

      advance(true);
      continue;
    }

    if (expect(TOK_JGE)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jgei;
        jgei.instruction = INS_JGEI;
        jgei.arguments[0] = g_symtable[index].address;
        jgei.arguments_len = 1;
        cvector_push_back(instructions, jgei);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jgei;
        jgei.instruction = INS_JGEI;
        jgei.arguments[0] = parse_immediate(g_current);
        jgei.arguments_len = 1;
        cvector_push_back(instructions, jgei);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jge;
      jge.instruction = INS_JGE;
      jge.arguments[0] = from_register(g_current.type);
      jge.arguments_len = 1;
      cvector_push_back(instructions, jge);

      advance(true);
      continue;
    }

    if (expect(TOK_JLE)) {
      advance(true);

      if (expect(TOK_IDENTIFIER)) {
        int index = symtable_find(g_current.span);

        if (index == -1) {
          fprintf(stderr, "ERROR: cannot find lable: ");
          span_print(stderr, g_current.span);
          fprintf(stderr, "\n");
          exit(1);
        }

        ParsedInstruction jlei;
        jlei.instruction = INS_JLEI;
        jlei.arguments[0] = g_symtable[index].address;
        jlei.arguments_len = 1;
        cvector_push_back(instructions, jlei);

        advance(true);
        continue;
      }

      if (is_immediate(g_current.type)) {
        ParsedInstruction jlei;
        jlei.instruction = INS_JLEI;
        jlei.arguments[0] = parse_immediate(g_current);
        jlei.arguments_len = 1;
        cvector_push_back(instructions, jlei);

        advance(true);
        continue;
      }

      if (!is_register(g_current.type)) {
        fprintf(stderr, "ERROR: expected register but got: ");
        span_print(stderr, g_current.span);
        fprintf(stderr, "\n");
        exit(1);
      }

      ParsedInstruction jle;
      jle.instruction = INS_JLE;
      jle.arguments[0] = from_register(g_current.type);
      jle.arguments_len = 1;
      cvector_push_back(instructions, jle);

      advance(true);
      continue;
    }
  }

  return instructions;
}

void parser_init(const char* input) {
  scanner_init(input);

  g_symtable_len = 0;
  g_symtable_cap = 10;
  g_symtable = malloc(sizeof(Symbol) * g_symtable_cap);

  if (!g_symtable) {
    fprintf(stderr, "ERROR: failed to allocate memory!\n");
    exit(1);
  }

  g_current = scanner_get_token();
  g_address = 0;
}

void parser_deinit() {
  free(g_symtable);
}
