#include <stdbool.h>

#include "alloc/arena/arena.h"
#include "lang/lexer/token.h"
#include "struct/alist.h"
#include "struct/q.h"

typedef struct {
  bool matched, has_children;
  Token *token;
  AList children;
} SyntaxTree;

typedef struct {
  __Arena st_arena;
  Q *tokens;
} Parser;

SyntaxTree NO_MATCH = {.matched = false, .has_children = false};
SyntaxTree MATCH_EPSILON = {
    .matched = true, .token = NULL, .has_children = false};

void parser_init(Parser *parser, Q *tokens) {
  parser->tokens = tokens;
  __arena_init(&parser->st_arena, sizeof(SyntaxTree), "SyntaxTree");
}

void parser_finalize(Parser *parser) { __arena_finalize(&parser->st_arena); }

SyntaxTree *_parser_create_st(Parser *parser) {
  return (SyntaxTree *)__arena_alloc(&parser->st_arena);
}

void _parser_delete_st(Parser *parser, SyntaxTree *st) {
  if (st->has_children) {
    AL_iter iter = alist_iter(&st->children);
    for (; al_has(&iter); al_inc(&iter)) {
      SyntaxTree *child = *(SyntaxTree **)al_value(&iter);
      if (&MATCH_EPSILON == child) {
        continue;
      }
      _parser_delete_st(parser, child);
    }
  }
  if (NULL != st->token) {
    Q_push(&parser->tokens, st->token);
  }
  __arena_dealloc(&parser->st_arena, st);
}

void _syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child) {
  if (!st->has_children) {
    alist_init(&st->children, SyntaxTree *, DEFAULT_ARRAY_SZ);
    st->has_children = true;
  }
  if (&MATCH_EPSILON == child) {
    return;
  }
  alist_append(&st->children, &child);
}

SyntaxTree *_match(Parser *parser) {
  SyntaxTree *st = _parser_create_st(parser);
  st->matched = true;
  st->token = Q_pop(parser->tokens);
  st->has_children = false;
  return st;
}

SyntaxTree *rule_identifier(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || TOKEN_WORD != token->type) {
    return &NO_MATCH;
  }
  return _match(parser);
}

inline SyntaxTree *_rule_or1__helper0(Parser *parser) {
  // and(...)
  SyntaxTree *tmp = _parser_create_st(parser);
  {
    // START token:KEYWORD_OR
    Token *token = _parser_next(parser);
    if (NULL == token || KEYWORD_OR != token->type) {
      _parser_delete_st(parser, tmp);
      return &NO_MATCH;
    }
    SyntaxTree *st = _match(parser);
    // END token:KEYWORD_OR
    _syntax_tree_add_child(tmp, st);
  }
  {
    // START rule:or
    SyntaxTree *st = rule_or(parser);
    if (!st->matched) {
      _parser_delete_st(parser, tmp);
      return &NO_MATCH;
    }
    // END rule:or
    _syntax_tree_add_child(tmp, st);
  }
  return tmp;
}

SyntaxTree *rule_or1(Parser *parser) {
  // or(...)
  SyntaxTree *st;
  {
    // START and(...)
    st = _rule_or1__helper0(parser);
    // END and(...)
    if (st->matched) {
      return st;
    }
  }
  {
    // START Epsilon
    return &MATCH_EPSILON;
    // END Epsilon
  }
}

SyntaxTree *rule_or(Parser *parser) {
  // and(...)
  SyntaxTree *st = _parser_create_st(parser);
  {
    // START rule:identifier
    SyntaxTree *st_child = rule_identifier(parser);
    if (!st->matched) {
      _parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    // END rule:identifier
    _syntax_tree_add_child(st, st_child);
  }
  {
    // START rule:or
    SyntaxTree *st_child = rule_or(parser);
    if (!st->matched) {
      _parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    // END rule:or
    _syntax_tree_add_child(st, st_child);
  }
}