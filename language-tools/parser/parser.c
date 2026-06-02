#include "language-tools/parser/parser.h"

#include <stdbool.h>

IMPL_ARRAYLIKE(SyntaxTreeArray, SyntaxTree *);

SyntaxTree NO_MATCH = {.matched = false, .has_children = false};
SyntaxTree MATCH_EPSILON = {
    .matched = true, .token = NULL, .has_children = false};

void parser_init(Parser *parser, RuleFn root, bool ignore_newline) {
  parser->root = root;
  parser->ignore_newline = ignore_newline;
  arena_init(&parser->st_arena, sizeof(SyntaxTree));
}

SyntaxTree *parser_parse(Parser *parser, TokenArray *tokens) {
  // Remove preceeding newlines.
  while (TokenArray_size(tokens) > 0 &&
         0 == strcmp("\n", TokenArray_get_unchecked(tokens, 0)->text)) {
    TokenArray_pop_front_unchecked(tokens);
  }
  parser->tokens = tokens;
  return parser->root(parser);
}

void parser_finalize(Parser *parser) { arena_clear(&parser->st_arena); }

Token *parser_next(Parser *parser) {
  if (TokenArray_is_empty(parser->tokens)) {
    return NULL;
  }
  if (parser->ignore_newline) {
    while (true) {
      if (TokenArray_is_empty(parser->tokens)) {
        return NULL;
      }
      Token *tok = TokenArray_get_unchecked(parser->tokens, 0);
      if (tok->type == 1 /* TOKEN_NEWLINE */) {
        TokenArray_pop_front_unchecked(parser->tokens);
      } else {
        break;
      }
    }
  }
  return TokenArray_get_unchecked(parser->tokens, 0);
}

Token *parser_remove(Parser *parser) {
  if (TokenArray_is_empty(parser->tokens)) {
    return NULL;
  }
  Token *token = TokenArray_pop_front_unchecked(parser->tokens);
  return token;
}

SyntaxTree *parser_create_st(Parser *parser, RuleFn rule_fn,
                             const char *production_name) {
  SyntaxTree *st = (SyntaxTree *)arena_malloc(&parser->st_arena);
  st->rule_fn = rule_fn;
  st->production_name = production_name;
  st->has_children = false;
  st->token = NULL;
  return st;
}

void parser_delete_st(Parser *parser, SyntaxTree *st) {
  if (st->has_children) {
    for (int i = SyntaxTreeArray_size(&st->children) - 1; i >= 0; --i) {
      SyntaxTree *child = SyntaxTreeArray_get_unchecked(&st->children, i);
      if (&MATCH_EPSILON == child) {
        continue;
      }
      parser_delete_st(parser, child);
    }
    SyntaxTreeArray_finalize(&st->children);
  }
  if (NULL != st->token) {
    TokenArray_push_front(parser->tokens, st->token);
  }
  arena_free(&parser->st_arena, st);
}

void syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child) {
  if (&MATCH_EPSILON == child) {
    return;
  }
  if (!st->has_children) {
    SyntaxTreeArray_init(&st->children);
    st->has_children = true;
  }
  SyntaxTreeArray_push_back(&st->children, child);
}

SyntaxTree *parser_prune_st(Parser *p, SyntaxTree *st) {
  if (!st->has_children || SyntaxTreeArray_size(&st->children) > 1) {
    return st;
  }
  SyntaxTree *child = SyntaxTreeArray_get_unchecked(&st->children, 0);
  SyntaxTreeArray_pop_back_unchecked(&st->children);
  parser_delete_st(p, st);
  return child;
}

SyntaxTree *match(Parser *parser, RuleFn rule_fn,
                  const char production_name[]) {
  SyntaxTree *st = parser_create_st(parser, rule_fn, production_name);
  st->matched = true;
  st->token = TokenArray_pop_front_unchecked(parser->tokens);
  st->has_children = false;
  return st;
}

void print_tabs_(FILE *file, int num_tabs) {
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
  print_tabs_(out, level);
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
  SyntaxTreeArrayIterator children;
  SyntaxTreeArray_iterator(&children, &st->children);
  for (; SyntaxTreeArray_has_next(&children); SyntaxTreeArray_next(&children)) {
    const SyntaxTree *st_child = *SyntaxTreeArray_value(&children);
    syntax_tree_print(st_child, level + 1, out);
    fprintf(out, "\n");
  }
  print_tabs_(out, level);
  fprintf(out, "}");
}

SyntaxTree *parser_prune_newlines(Parser *p, SyntaxTree *st) {
  if (&NO_MATCH == st || !st->has_children) {
    return st;
  }
  int i;
  for (i = SyntaxTreeArray_size(&st->children) - 1; i >= 0; --i) {
    SyntaxTree *st_child = SyntaxTreeArray_get_unchecked(&st->children, i);
    if (st_child->has_children) {
      *SyntaxTreeArray_mutable_ref_unchecked(&st->children, i) =
          parser_prune_newlines(p, st_child);
    } else if (1 /*TOKEN_NEWLINE*/ == st_child->token->type) {
      arena_free(&p->st_arena, st_child);
      SyntaxTreeArray_remove_unchecked(&st->children, i);
    }
  }
  if (1 == SyntaxTreeArray_size(&st->children)) {
    SyntaxTree *child = SyntaxTreeArray_get_unchecked(&st->children, 0);
    SyntaxTreeArray_finalize(&st->children);
    arena_free(&p->st_arena, st);
    return child;
  }
  return st;
}