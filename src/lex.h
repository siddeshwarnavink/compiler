#ifndef LEX_H
#define LEX_H

#include <stdio.h>

#define MAX_SYMBOL_LEN 1024

typedef struct lex {
  const char *file_path;
  FILE *file;
  long int_val;
  char *str_val;
  int str_val_size;
  int str_val_capacity;
  int line;
  int col;
} lex_t;

typedef enum token {
  T_EOF = 256,
  T_SYMBOL,
  T_STRLIT,
  T_INTLIT,
  T_I32,
  T_LAST
} token_t;

int lex_init(lex_t *l, const char *file_path);

token_t lex_next(lex_t *l);

void lex_kind_label(lex_t *l, token_t t, char *buf);

void lex_report_err(lex_t *lexer, const char *fmt, ...);

void lex_free(lex_t *l);

#endif /* ifndef LEX_H */
