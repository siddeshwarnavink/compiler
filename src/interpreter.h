#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"

typedef struct interpreter_functions {
  char *key;
  ast_node_t *value;
} interpreter_functions_t;

void interpreter_run(ast_node_da_t *list);

#endif /* ifndef INTERPRETER_H */
