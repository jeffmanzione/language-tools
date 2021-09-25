#ifndef LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_
#define LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_

#include <stdbool.h>
#include <stdint.h>

#include "debug/debug.h"
#include "lang/lexer/token.h"
#include "lang/parser/parser.h"
#include "struct/alist.h"
#include "struct/map.h"

typedef struct {
  RuleFn type;
  const char *rule_name;
  void *expression;
} ExpressionTree;

typedef ExpressionTree *(*Populator)(const SyntaxTree *tree);
typedef void (*EDeleter)(ExpressionTree *tree);

#define DefineExpression(name)                                            \
  typedef struct _Expression_##name Expression_##name;                    \
  ExpressionTree *Populate_##name(const SyntaxTree *tree);                \
  void Transform_##name(const SyntaxTree *tree, Expression_##name *name); \
  void Delete_##name(ExpressionTree *tree);                               \
  void Delete_##name##_inner(Expression_##name *name);                    \
  struct _Expression_##name

#define ImplPopulate(name, stree_input)             \
  ExpressionTree *Populate_##name(stree_input) {    \
    ExpressionTree *etree = ALLOC2(ExpressionTree); \
    etree->type = rule_##name;                      \
    etree->rule_name = #name;                       \
    etree->expression = ALLOC(Expression_##name);   \
    Transform_##name(stree, etree->expression);     \
    return etree;                                   \
  }                                                 \
  void Transform_##name(stree_input, Expression_##name *name)

#define ImplDelete(name)                     \
  void Delete_##name(ExpressionTree *tree) { \
    Delete_##name##_inner(tree->expression); \
  }                                          \
  void Delete_##name##_inner(Expression_##name *name)

#define Register(name)                                    \
  {                                                       \
    map_insert(populators, rule_##name, Populate_##name); \
    map_insert(deleters, rule_##name, Delete_##name);     \
  }

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_ */