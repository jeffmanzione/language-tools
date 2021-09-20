#include "lang/parser/parser.h"

#include <stdbool.h>

void parser_init(Parser *parser, RuleFn root) {
  parser->root = root;
  __arena_init(&parser->st_arena, sizeof(SyntaxTree), "SyntaxTree");
}

SyntaxTree *parser_parse(Parser *parser, Q *tokens) {
  parser->tokens = tokens;
  return parser->root(parser);
}

void parser_finalize(Parser *parser) { __arena_finalize(&parser->st_arena); }

Token *parser_next(Parser *parser) {
  if (Q_is_empty(parser->tokens)) {
    return NULL;
  }
  return Q_pop(parser->tokens);
}

SyntaxTree *parser_create_st(Parser *parser) {
  return (SyntaxTree *)__arena_alloc(&parser->st_arena);
}

void parser_delete_st(Parser *parser, SyntaxTree *st) {
  if (st->has_children) {
    AL_iter iter = alist_iter(&st->children);
    for (; al_has(&iter); al_inc(&iter)) {
      SyntaxTree *child = *(SyntaxTree **)al_value(&iter);
      if (&MATCH_EPSILON == child) {
        continue;
      }
      parser_delete_st(parser, child);
    }
  }
  if (NULL != st->token) {
    Q_push(parser->tokens, st->token);
  }
  __arena_dealloc(&parser->st_arena, st);
}

void syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child) {
  if (!st->has_children) {
    alist_init(&st->children, SyntaxTree *, DEFAULT_ARRAY_SZ);
    st->has_children = true;
  }
  if (&MATCH_EPSILON == child) {
    return;
  }
  alist_append(&st->children, &child);
}

SyntaxTree *match(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  st->matched = true;
  st->token = Q_pop(parser->tokens);
  st->has_children = false;
  return st;
}
