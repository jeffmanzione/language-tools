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

#define DefineExpression(name)                                           \
  typedef struct _Expression_##name Expression_##name;                   \
  ExpressionTree *Populate_##name(const SyntaxTree *tree,                \
                                  SemanticAnalyzer *analyzer);           \
  void Transform_##name(const SyntaxTree *tree, Expression_##name *name, \
                        SemanticAnalyzer *analyzer);                     \
  void Delete_##name(ExpressionTree *tree, SemanticAnalyzer *analyzer);  \
  void Delete_##name##_inner(Expression_##name *name,                    \
                             SemanticAnalyzer *analyzer);                \
  struct _Expression_##name

#define ImplPopulate(name, stree_input, analyzer_input)          \
  ExpressionTree *Populate_##name(stree_input, analyzer_input) { \
    ExpressionTree *etree = ALLOC2(ExpressionTree);              \
    etree->type = rule_##name;                                   \
    etree->rule_name = #name;                                    \
    etree->expression = ALLOC(Expression_##name);                \
    Transform_##name(stree, etree->expression, analyzer);        \
    return etree;                                                \
  }                                                              \
  void Transform_##name(stree_input, Expression_##name *name, analyzer_input)

#define ImplDelete(name, analyzer_input)                     \
  void Delete_##name(ExpressionTree *tree, analyzer_input) { \
    Delete_##name##_inner(tree->expression, analyzer);       \
  }                                                          \
  void Delete_##name##_inner(Expression_##name *name, analyzer_input)

#define Register(name)                                    \
  {                                                       \
    map_insert(populators, rule_##name, Populate_##name); \
    map_insert(deleters, rule_##name, Delete_##name);     \
  }

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_ */