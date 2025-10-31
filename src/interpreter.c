#include "interpreter.h"

#include <assert.h>
#include <stdio.h>

// #include "arena.h"
#include "ast.h"
#include "stb_ds.h"

// static Arena interpreter_arena = {0};
static interpreter_functions_t *functions;
static interpreter_variables_t *variables;

static void _interpreter_execute(ast_node_t *node);
static void _builtin_printf(ast_node_da_t *args);

void interpreter_run(ast_node_da_t *list) {
  for (size_t i = 0; i < list->count; ++i) _interpreter_execute(list->items[i]);

  if (shgetp(functions, "main") == NULL)
    fprintf(stderr, "Error: Missing entry point main.\n");

  shfree(functions);
  shfree(variables);
  // arena_free(&interpreter_arena);
}

void _interpreter_execute(ast_node_t *node) {
  assert(A_LAST == 7 && "Implementation missing");

  switch (node->kind) {
    case A_STRLIT:
      break;

    case A_MAIN: {
      shput(functions, "main", node);
      if (node->data.fundef.body) _interpreter_execute(node->data.fundef.body);
    } break;

    case A_SCOPE: {
      ast_node_da_t *stmts = &node->data.statements;
      for (size_t i = 0; i < stmts->count; ++i)
        _interpreter_execute(stmts->items[i]);
    } break;

    case A_VAR_DECLARE: {
      const char *name = node->data.vardeclare.name;
      shput(variables, name, node->data.vardeclare.value);
    } break;

    case A_FUNCALL: {
      const char *name = node->data.funcall.name;

      if (strcmp(name, "printf") == 0) {
        _builtin_printf(&node->data.funcall.args);
        return;
      }

      interpreter_functions_t *func = shgetp(functions, name);
      if (func == NULL) {
        fprintf(stderr, "Error: Undefined function '%s'\n", name);
        return;
      }

      if (func->value->data.fundef.body)
        _interpreter_execute(func->value->data.fundef.body);
    } break;

    default:
      fprintf(stderr, "Error: Unknown AST node kind: %d\n", node->kind);
      break;
  }
}

void _builtin_printf(ast_node_da_t *args) {
  if (args->count < 1) return;

  ast_node_t *fmt_node = args->items[0];
  if (fmt_node->kind != A_STRLIT) {
    fprintf(stderr, "Error: printf expects string literal as first argument\n");
    return;
  }
  printf("%s", fmt_node->data.str_val);
}
