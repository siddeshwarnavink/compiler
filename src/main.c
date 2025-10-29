#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "ast.h"
#include "interpreter.h"

int main(int argc, char **argv) {
  Arena arena = {0};

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <input>\n", argv[0]);
    fprintf(stderr, "Error: Missing input file path");
    return 1;
  }

  lex_t lexer = {0};
  if (lex_init(&lexer, argv[1]) < 0) {
    perror("lex_init");
    return 1;
  }

  parser_t p = {0};
  parser_init(&p, &lexer);

  ast_node_t *node;
  ast_node_da_t node_list = {0};
  while ((node = parser_next(&p)) != NULL) {
    arena_da_append(&arena, &node_list, node);
    // parser_print_node(node);
  }

  interpreter_run(&node_list);

  parser_free(&p);
  lex_free(&lexer);
  arena_free(&arena);

  return 0;
}
