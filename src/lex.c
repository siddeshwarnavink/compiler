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
  assert(T_LAST == 261 && "Implementation missing");

  char ch = fgetc(l->file), chnext;
  l->str_val_size = 0;
  l->int_val = 0;

  if (ch == EOF) return T_EOF;

  // Skip whitespace
  if (isspace(ch)) {
    while ((ch = fgetc(l->file)) != EOF) {
      if (ch == '\n') {
        l->line++;
        l->col = 0;
      } else {
        l->col++;
      }

      if (!isspace(ch)) break;
    }
  }

  // Skip comments
  if (ch == '/') {
    chnext = fgetc(l->file);
    if (chnext == '/') {
      while ((ch = fgetc(l->file)) != EOF && ch != '\n') {
        l->col++;
      }
      if (ch == '\n') {
        l->line++;
        l->col = 0;
      }
      return lex_next(l);
    } else if (chnext == '*') {
      while ((ch = fgetc(l->file)) != EOF) {
        if (ch == '\n') {
          l->line++;
          l->col = 0;
        }
        if (ch == '*') {
          if ((ch = fgetc(l->file)) == '/')
            break;
          else
            ungetc(ch, l->file);
        }
        l->col++;
      }
      return lex_next(l);
    } else {
      ungetc(chnext, l->file);
    }
  }

  // String literal
  if (ch == '"') {
    l->col++;
    while ((ch = fgetc(l->file)) != EOF) {
      if (ch == '"') break;
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

  // Decimal Number literal
  if (isdigit(ch)) {
    l->int_val = ch - '0';
    l->col++;
    while ((ch = fgetc(l->file)) != EOF) {
      if (!isdigit(ch)) {
        fseek(l->file, -1, SEEK_CUR);
        l->col--;
        break;
      }
      l->col++;
      l->int_val *= 10;
      l->int_val += ch - '0';
    }
    return T_INTLIT;
  }

  // TODO: Support Hexadecimal literals.

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

  if (strncmp(l->str_val, "i32", l->str_val_size) == 0) return T_I32;

  return T_SYMBOL;
}

token_t lex_peek(lex_t *l) {
  fpos_t pos_bak;
  int line_bak = l->line;
  int col_bak = l->col;

  fgetpos(l->file, &pos_bak);
  token_t token = lex_next(l);

  l->line = line_bak;
  l->col = col_bak;
  fsetpos(l->file, &pos_bak);

  return token;
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

void lex_kind_label(lex_t *l, token_t t, char *buf) {
  assert(T_LAST == 261 && "Implementation missing");

  if (t < 256) {
    sprintf(buf, "'%c'", (char)t);
    return;
  }

  switch (t) {
    case T_EOF:
      sprintf(buf, "T_EOF");
      break;
    case T_SYMBOL:
      sprintf(buf, "T_SYMBOL(%.*s)", l->str_val_size, l->str_val);
      break;
    case T_STRLIT:
      sprintf(buf, "T_STRLIT(%.*s)", l->str_val_size, l->str_val);
      break;
    case T_INTLIT:
      sprintf(buf, "T_INTLIT(%ld)", l->int_val);
      break;
    case T_LAST:
      sprintf(buf, "T_LAST");
      break;
    case T_I32:
      sprintf(buf, "T_I32");
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
