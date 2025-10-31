#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"
#include "ast.h"
#include "interpreter.h"

typedef enum compiler_action {
  CA_LEXDUMP = 0,
  CA_ASTDUMP,
  CA_INTERPRET,
} compiler_action_t;

static inline char* shift(char*** argv) { return **argv ? *(*argv)++ : NULL; }

int main(int argc, char** argv) {
  Arena arena = {0};

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <input>\n", argv[0]);
    fprintf(stderr, "Error: Missing input file path");
    return 1;
  }

  ++argv;
  char* file_input = shift(&argv);
  compiler_action_t action = CA_INTERPRET;

  char* flag;
  while ((flag = shift(&argv)) != NULL) {
    if      (strcmp(flag, "-lexdump") == 0) action = CA_LEXDUMP;
    else if (strcmp(flag, "-astdump") == 0) action = CA_ASTDUMP;
  }

  lex_t lexer = {0};
  if (lex_init(&lexer, file_input) < 0) {
    perror("lex_init");
    return 1;
  }

  if (action == CA_LEXDUMP) {
    token_t token;
    while ((token = lex_next(&lexer)) != T_EOF) {
      char buf[256];
      lex_kind_label(&lexer, token, buf);
      printf("%s\n", buf);
    }
  } else {
    parser_t p = {0};
    parser_init(&p, &lexer);

    ast_node_t* node;
    ast_node_da_t node_list = {0};
    while ((node = parser_next(&p)) != NULL) {
      if (action == CA_ASTDUMP) parser_print_node(node);
      else arena_da_append(&arena, &node_list, node);
    }

    if (action == CA_INTERPRET) interpreter_run(&node_list);

    parser_free(&p);
  }

  lex_free(&lexer);
  arena_free(&arena);

  return 0;
}
