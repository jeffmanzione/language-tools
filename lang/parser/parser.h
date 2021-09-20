#ifndef LANGUAGE_TOOLS_LANG_PARSER_PARSER_H_
#define LANGUAGE_TOOLS_LANG_PARSER_PARSER_H_

#include "alloc/arena/arena.h"
#include "lang/lexer/token.h"
#include "struct/alist.h"
#include "struct/q.h"

typedef struct _SyntaxTree SyntaxTree;
typedef struct _Parser Parser;

typedef SyntaxTree *(*RuleFn)(Parser *parser);

struct _SyntaxTree {
  bool matched, has_children;
  Token *token;
  AList children;
};

struct _Parser {
  __Arena st_arena;
  RuleFn root;
  Q *tokens;
};

SyntaxTree NO_MATCH = {.matched = false, .has_children = false};
SyntaxTree MATCH_EPSILON = {
    .matched = true, .token = NULL, .has_children = false};

void parser_init(Parser *parser, RuleFn root);
SyntaxTree *parser_parse(Parser *parser, Q *tokens);
void parser_finalize(Parser *parser);
Token *parser_next(Parser *parser);
SyntaxTree *parser_create_st(Parser *parser);
void parser_delete_st(Parser *parser, SyntaxTree *st);
void syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child);
SyntaxTree *match(Parser *parser);

#endif /* LANGUAGE_TOOLS_LANG_PARSER_PARSER_H_ */