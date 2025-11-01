#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

static void _ensure_capacity(lex_t *l, int needed) {
  if (l->str_val_size + needed >= l->str_val_capacity) {
    l->str_val_capacity *= 2;
    l->str_val = realloc(l->str_val, l->str_val_capacity);
  }
}

static inline int _fpeek(FILE *f) {
    int ch = fgetc(f);
    if (ch != EOF) ungetc(ch, f);
    return ch;
}

int lex_init(lex_t *l, const char *file_path) {
  l->file_path = file_path;
  l->str_val = malloc(LEX_MAX_SYMBOL_LEN);
  if (!l->str_val) return -1;
  l->str_val_capacity = LEX_MAX_SYMBOL_LEN;

  l->file = fopen(file_path, "r");
  if (!l->file) return -1;

  return 1;
}

token_t lex_next(lex_t *l) {
  assert(T_LAST == 261 && "Implementation missing");

  char ch = fgetc(l->file);
  l->str_val_size = 0;
  l->int_val = 0;

  if (ch == EOF) return T_EOF;

  // Skip whitespace
  if (isspace(ch)) {
    if (ch == '\n') {
      l->line++;
      l->col = 0;
    } else {
      l->col++;
    }

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
  if (ch == '/' && _fpeek(l->file) == '/') {
    fgetc(l->file);
    l->col += 2;
    while ((ch = fgetc(l->file)) != EOF && ch != '\n') l->col++;
    if (ch == '\n') {
      l->line++;
      l->col = 0;
    }
    return lex_next(l);
  }
  else if (ch == '/' && _fpeek(l->file) == '*') {
    fgetc(l->file);
    l->col += 2;
    while ((ch = fgetc(l->file)) != EOF) {
      if (ch == '\n') {
        l->line++;
        l->col = 0;
      }
      if (ch == '*' && _fpeek(l->file) == '/') {
        fgetc(l->file);
        l->col++;
        return lex_next(l);
      }
      l->col++;
    }
    return lex_next(l);
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
  if (isalpha(ch) || ch == '_') {
    while (ch != EOF && (isalnum(ch) || ch == '_')) {
      _ensure_capacity(l, 1);
      l->str_val[l->str_val_size++] = ch;
      l->col++;
      ch = fgetc(l->file);
    }

    l->str_val[l->str_val_size] = '\0';
    ungetc(ch, l->file);

    if (strcmp(l->str_val, "i32") == 0) return T_I32;

    return T_SYMBOL;
  }

  l->col++;
  return T_EOF;
}

token_t lex_peek(lex_t *l) {
  fpos_t pos_bak;
  int line_bak = l->line;
  int col_bak = l->col;

  char *str_val_bak = NULL;
  size_t str_val_size_bak = l->str_val_size;

  if (l->str_val_size > 0) {
    str_val_bak =  malloc(l->str_val_capacity);
    if (str_val_bak) strncpy(str_val_bak, l->str_val, l->str_val_size);
  }

  fgetpos(l->file, &pos_bak);
  token_t token = lex_next(l);

  l->line = line_bak;
  l->col = col_bak;
  fsetpos(l->file, &pos_bak);

  if (str_val_bak) {
    strncpy(l->str_val, str_val_bak, str_val_size_bak);
    l->str_val_size = str_val_size_bak;
  }

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
      sprintf(buf, "T_SYMBOL(%.*s)", (int)l->str_val_size, l->str_val);
      break;
    case T_STRLIT:
      sprintf(buf, "T_STRLIT(%.*s)", (int)l->str_val_size, l->str_val);
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
