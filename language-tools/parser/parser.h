#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_PARSER_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_PARSER_H_

#include <stdio.h>

#include "c-data-structures/arraylike.h"
#include "language-tools/lexer/token.h"
#include "rzalloc/rzalloc.h"

typedef struct SyntaxTree_ SyntaxTree;
typedef struct Parser_ Parser;

typedef SyntaxTree *(*RuleFn)(Parser *parser);

DEFINE_ARRAYLIKE(SyntaxTreeArray, SyntaxTree *);

struct SyntaxTree_ {
  RuleFn rule_fn;
  const char *production_name;
  bool matched, has_children;
  Token *token;
  SyntaxTreeArray children;
};

struct Parser_ {
  RzallocArena st_arena;
  RuleFn root;
  TokenArray *tokens;
  bool ignore_newline;
};

extern SyntaxTree NO_MATCH;
extern SyntaxTree MATCH_EPSILON;

void parser_init(Parser *parser, RuleFn root, bool ignore_newline);
SyntaxTree *parser_parse(Parser *parser, TokenArray *tokens);
void parser_finalize(Parser *parser);
Token *parser_next(Parser *parser);
SyntaxTree *parser_create_st(Parser *parser, RuleFn rule_fn,
                             const char production_name[]);
void parser_delete_st(Parser *parser, SyntaxTree *st);
SyntaxTree *parser_prune_st(Parser *p, SyntaxTree *st);
void syntax_tree_add_child(SyntaxTree *st, SyntaxTree *child);
SyntaxTree *match(Parser *parser, RuleFn rule_fn, const char production_name[]);

void syntax_tree_print(const SyntaxTree *st, int level, FILE *out);

SyntaxTree *parser_prune_newlines(Parser *p, SyntaxTree *st);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_PARSER_H_ */