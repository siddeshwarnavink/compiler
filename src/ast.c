#include "ast.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "arena.h"

static bool _parser_expect_next(parser_t *p, token_t t);

static void _throw_expect_but_got(parser_t *p, token_t t1, token_t t2);

static Arena ast_arena = {0};

void parser_init(parser_t *p, lex_t *lexer) {
  p->lexer = lexer;
  p->current_token = lex_next(lexer);
}

ast_node_t *parser_next(parser_t *p) {
  assert(A_LAST == 5 && "Implementation missing");

  if (!p || p->current_token == T_EOF) return NULL;

  ast_node_t *node = arena_alloc(&ast_arena, sizeof(ast_node_t));

  // String literal
  if (p->current_token == T_STRLIT) {
    node->kind = A_STRLIT;

    node->data.str_val = arena_alloc(&ast_arena, p->lexer->str_val_size + 1);
    strncpy(node->data.str_val, p->lexer->str_val, p->lexer->str_val_size);
    node->data.str_val[p->lexer->str_val_size] = '\0';

    p->current_token = lex_next(p->lexer);

    return node;
  }

  // Main function
  if (p->current_token == T_SYMBOL &&
      strncmp("main", p->lexer->str_val, 4) == 0) {
    node->data.fundef.name =
        arena_alloc(&ast_arena, p->lexer->str_val_size + 1);
    strncpy(node->data.fundef.name, p->lexer->str_val, p->lexer->str_val_size);
    node->data.fundef.name[p->lexer->str_val_size] = '\0';

    if (!_parser_expect_next(p, '(')) return NULL;
    if (!_parser_expect_next(p, ')')) return NULL;

    p->current_token = lex_next(p->lexer);

    node->kind = A_MAIN;
    node->data.fundef.body = parser_next(p);
    return node;
  }

  // Scope
  else if (p->current_token == '{') {
    node->kind = A_SCOPE;

    p->current_token = lex_next(p->lexer);
    while (p->current_token != '}') {
      ast_node_t *arg = parser_next(p);
      arena_da_append(&ast_arena, &node->data.statements, arg);
    }

    p->current_token = lex_next(p->lexer);

    return node;
  }

  // Function call
  else if (p->current_token == T_SYMBOL) {
    node->kind = A_FUNCALL;
    node->data.funcall.name =
        arena_alloc(&ast_arena, p->lexer->str_val_size + 1);
    strncpy(node->data.funcall.name, p->lexer->str_val, p->lexer->str_val_size);
    node->data.funcall.name[p->lexer->str_val_size] = '\0';

    if (!_parser_expect_next(p, '(')) return NULL;

    p->current_token = lex_next(p->lexer);
    while (p->current_token != ')') {
      arena_da_append(&ast_arena, &node->data.funcall.args, parser_next(p));

      if (p->current_token == ',') {
        p->current_token = lex_next(p->lexer);
      } else if (p->current_token != ')') {
        _throw_expect_but_got(p, ',', ')');
        return NULL;
      }
    }

    if (!_parser_expect_next(p, ';')) return NULL;

    p->current_token = lex_next(p->lexer);

    return node;
  }

  char buf[32];
  lex_kind_label(p->current_token, buf);
  printf("[info] lex token: %s\n", buf);
  assert(0 && "Unhandled token");

  return NULL;
}

void parser_print_node(ast_node_t *node) {
  assert(A_LAST == 5 && "Implementation missing");

  if (!node) {
    printf("nil");
    return;
  }

  switch (node->kind) {
    case A_STRLIT:
      printf("(str %s)", node->data.str_val);
      break;

    case A_FUNCALL:
      printf("(call %s", node->data.funcall.name);
      for (size_t i = 0; i < node->data.funcall.args.count; i++) {
        printf(" ");
        parser_print_node(node->data.funcall.args.items[i]);
      }
      printf(")");
      break;

    case A_SCOPE:
      printf("(scope");
      for (size_t i = 0; i < node->data.statements.count; i++) {
        printf("\n  ");
        parser_print_node(node->data.statements.items[i]);
      }
      printf(")");
      break;

    case A_MAIN:
    case A_FUNDEF:
      printf("(def %s (", node->data.fundef.name);
      for (size_t i = 0; i < node->data.fundef.args.count; i++) {
        if (i > 0) printf(" ");
        parser_print_node(node->data.fundef.args.items[i]);
      }
      printf(") ");
      parser_print_node(node->data.fundef.body);
      printf(")");
      break;

    default:
      printf("[info] ast node kind: %d\n", node->kind);
      assert(0 && "unknown kind");
      break;
  }
}

void parser_free(parser_t *p) {
  (void)p;
  arena_free(&ast_arena);
  p = NULL;
}

bool _parser_expect_next(parser_t *p, token_t t) {
  p->current_token = lex_next(p->lexer);
  if (p->current_token != t) {
    _throw_expect_but_got(p, t, p->current_token);
    return false;
  }
  return true;
}

static void _throw_expect_but_got(parser_t *p, token_t t1, token_t t2) {
  char buf1[32], buf2[32];
  lex_kind_label(t1, buf1);
  lex_kind_label(t2, buf2);
  lex_report_err(p->lexer, "Expected token %s but got %s\n", buf1, buf2);
  lex_free(p->lexer);
  parser_free(p);
  exit(1);
}
