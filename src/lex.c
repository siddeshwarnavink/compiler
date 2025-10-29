#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

static void _ensure_capacity(lex_t *l, int needed);

int lex_init(lex_t *l, const char *file_path) {
  l->file_path = file_path;
  l->str_val = malloc(MAX_SYMBOL_LEN);
  if (!l->str_val) return -1;
  l->str_val_capacity = MAX_SYMBOL_LEN;

  l->file = fopen(file_path, "r");
  if (!l->file) return -1;

  return 1;
}

token_t lex_next(lex_t *l) {
  assert(T_LAST == 260 && "Implementation missing");

  char ch;
  l->str_val_size = 0;

  // Skip whitespace
  while ((ch = fgetc(l->file)) != EOF) {
    if (ch == '\n') {
      l->line++;
      l->col = 0;
    } else {
      l->col++;
    }

    if (!isspace(ch)) break;
  }

  if (ch == EOF) return T_EOF;

  // String literal
  if (ch == '"') {
    l->col++;
    while ((ch = fgetc(l->file)) != EOF && ch != '"') {
      l->col++;

      if (ch == '\\') {
        ch = fgetc(l->file);
        l->col++;
        switch (ch) {
          case 'n':
            ch = '\n';
            break;
          case 't':
            ch = '\t';
            break;
          case 'r':
            ch = '\r';
            break;
          case '\\':
            ch = '\\';
            break;
          case '"':
            ch = '"';
            break;
          default:
            break;
        }
      }
      _ensure_capacity(l, 1);
      l->str_val[l->str_val_size++] = ch;
    }
    l->str_val[l->str_val_size] = '\0';
    return T_STRLIT;
  }

  // TODO: Support for number literal

  // Operators/punctuation
  if (strchr("(){}[]<>.,;:=+-*/!&|", ch)) {
    l->str_val[0] = ch;
    l->str_val[1] = '\0';
    l->str_val_size = 1;
    return ch;
  }

  // Symbol
  while (ch != EOF && !isspace(ch) && !strchr("(){}[]<>.,;:\"", ch)) {
    _ensure_capacity(l, 1);
    l->str_val[l->str_val_size++] = ch;
    ch = fgetc(l->file);
    l->col++;
  }

  l->str_val[l->str_val_size] = '\0';
  ungetc(ch, l->file);
  l->col--;

  return T_SYMBOL;
}

void lex_report_err(lex_t *lexer, const char *fmt, ...) {
  va_list args;

  fprintf(stderr, "%s:%d:%d: error: ",
          lexer->file_path ? lexer->file_path : "<unknown>", lexer->line,
          lexer->col);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");
}

void lex_kind_label(token_t t, char *buf) {
  assert(T_LAST == 260 && "Implementation missing");

  if (t < 256) {
    sprintf(buf, "'%c'", (char)t);
    return;
  }

  switch (t) {
    case T_EOF:
      sprintf(buf, "T_EOF");
      break;
    case T_SYMBOL:
      sprintf(buf, "T_SYMBOL");
      break;
    case T_STRLIT:
      sprintf(buf, "T_STRLIT");
      break;
    case T_INTLIT:
      sprintf(buf, "T_INTLIT");
      break;
    case T_LAST:
      sprintf(buf, "T_LAST");
      break;
    default:
      assert(0 && "Unhandled token");
      break;
  }
}

void lex_free(lex_t *l) { free(l->str_val); }

void _ensure_capacity(lex_t *l, int needed) {
  if (l->str_val_size + needed >= l->str_val_capacity) {
    l->str_val_capacity *= 2;
    l->str_val = realloc(l->str_val, l->str_val_capacity);
  }
}
