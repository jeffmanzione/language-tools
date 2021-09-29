#include "lang/parser/parser.h"

#include <stdbool.h>

#include "struct/alist.h"

SyntaxTree NO_MATCH = {.matched = false, .has_children = false};
SyntaxTree MATCH_EPSILON = {
    .matched = true, .token = NULL, .has_children = false};

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
  return Q_get(parser->tokens, 0);
}

Token *parser_remove(Parser *parser) {
  if (Q_is_empty(parser->tokens)) {
    return NULL;
  }
  Token *token = Q_pop(parser->tokens);
  return token;
}

SyntaxTree *parser_create_st(Parser *parser, RuleFn rule_fn,
                             const char *production_name) {
  SyntaxTree *st = (SyntaxTree *)__arena_alloc(&parser->st_arena);
  st->rule_fn = rule_fn;
  st->production_name = production_name;
  st->has_children = false;
  st->token = NULL;
  return st;
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
    alist_finalize(&st->children);
  }
  if (NULL != st->token) {
    Q_push(parser->tokens, st->token);
  }
  __arena_dealloc(&parser->st_arena, st);
}

void syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child) {
  if (&MATCH_EPSILON == child) {
    return;
  }
  if (!st->has_children) {
    alist_init(&st->children, SyntaxTree *, DEFAULT_ARRAY_SZ);
    st->has_children = true;
  }
  alist_append(&st->children, &child);
}

SyntaxTree *match(Parser *parser, RuleFn rule_fn,
                  const char production_name[]) {
  SyntaxTree *st = parser_create_st(parser, rule_fn, production_name);
  st->matched = true;
  st->token = Q_pop(parser->tokens);
  st->has_children = false;
  return st;
}

void _print_tabs(FILE *file, int num_tabs) {
  int i;
  for (i = 0; i < num_tabs; i++) {
    fprintf(file, "  ");
  }
}

void syntax_tree_print(const SyntaxTree *st, int level, FILE *out) {
  if (&NO_MATCH == st) {
    fprintf(out, "NO_MATCH");
    return;
  }
  _print_tabs(out, level);
  if (NULL != st->production_name) {
    fprintf(out, "[%s] ", st->production_name);
  }
  if (!st->has_children) {
    if (&MATCH_EPSILON == st) {
      fprintf(out, "E");
    } else {
      const char *text = st->token->text;
      if ('\n' == text[0]) {
        fprintf(out, "\\n");
      } else {
        fprintf(out, "\"%s\"", text);
      }
    }
    return;
  }
  fprintf(out, "{\n");
  AL_iter children = alist_iter(&st->children);
  for (; al_has(&children); al_inc(&children)) {
    const SyntaxTree *st_child = *(SyntaxTree **)al_value(&children);
    syntax_tree_print(st_child, level + 1, out);
    fprintf(out, "\n");
  }
  _print_tabs(out, level);
  fprintf(out, "}");
}