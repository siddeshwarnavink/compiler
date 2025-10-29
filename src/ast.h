#ifndef AST_H
#define AST_H

#include "lex.h"

typedef enum ast_kind {
  A_STRLIT,
  A_MAIN,
  A_SCOPE,
  A_FUNCALL,
  A_FUNDEF,
  A_LAST
} ast_kind_t;

typedef struct ast_node ast_node_t;

typedef struct ast_node_da {
  size_t count, capacity;
  struct ast_node **items;
} ast_node_da_t;

typedef struct ast_node {
  ast_kind_t kind;
  union {
    // String literal
    char *str_val;

    // Function call
    struct {
      char *name;
      ast_node_da_t args;
    } funcall;

    // Scope
    ast_node_da_t statements;

    // Function / Main
    struct {
      char *name;
      ast_node_da_t args;
      struct ast_node *body;
    } fundef;
  } data;
} ast_node_t;

typedef struct parser {
  lex_t *lexer;
  token_t current_token;
} parser_t;

void parser_init(parser_t *p, lex_t *lexer);

ast_node_t *parser_next(parser_t *p);

void parser_print_node(ast_node_t *node);

void parser_free(parser_t *p);

#endif /* ifndef AST_H */
