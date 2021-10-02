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

#define DEFINE_EXPRESSION(name)                                          \
  typedef struct _Expression_##name Expression_##name;                   \
  ExpressionTree *Populate_##name(const SyntaxTree *tree,                \
                                  SemanticAnalyzer *analyzer);           \
  void Transform_##name(const SyntaxTree *tree, Expression_##name *name, \
                        SemanticAnalyzer *analyzer);                     \
  void Delete_##name(ExpressionTree *tree, SemanticAnalyzer *analyzer);  \
  void Delete_##name##_inner(Expression_##name *name,                    \
                             SemanticAnalyzer *analyzer);                \
  struct _Expression_##name

#define POPULATE_IMPL(name, stree_input, analyzer_input)         \
  ExpressionTree *Populate_##name(stree_input, analyzer_input) { \
    ExpressionTree *etree = ALLOC2(ExpressionTree);              \
    etree->type = rule_##name;                                   \
    etree->rule_name = #name;                                    \
    etree->expression = ALLOC(Expression_##name);                \
    Transform_##name(stree, etree->expression, analyzer);        \
    return etree;                                                \
  }                                                              \
  void Transform_##name(stree_input, Expression_##name *name, analyzer_input)

#define DELETE_IMPL(name, analyzer_input)                    \
  void Delete_##name(ExpressionTree *tree, analyzer_input) { \
    Delete_##name##_inner(tree->expression, analyzer);       \
  }                                                          \
  void Delete_##name##_inner(Expression_##name *name, analyzer_input)

#define REGISTER_EXPRESSION(name)                         \
  {                                                       \
    map_insert(populators, rule_##name, Populate_##name); \
    map_insert(deleters, rule_##name, Delete_##name);     \
  }

#define EXPECT_TYPE(stree, type)    \
  if (stree->rule_fn != type) {     \
    ERROR("Expected type: " #type); \
  }

#define IS_SYNTAX(stree, type) ((stree->rule_fn) == (type))

#define IS_TOKEN(stree) ((NULL != (stree)) && (NULL != (stree->token)))

#define TOKEN_TEXT_FOR(stree) \
  (((NULL != (stree)) && (NULL != (stree->token))) ? stree->token->text : NULL)

#define CHILD_SYNTAX_AT(stree, index)                         \
  (alist_len(&(stree)->children) > index                      \
       ? *(SyntaxTree **)alist_get(&(stree)->children, index) \
       : NULL)

#define IS_EXPRESSION(etree, typ) \
  ((NULL != (etree)) && ((rule_##typ) == (etree)->type))

#define EXTRACT_EXPRESSION(etree, type)                                      \
  (IS_EXPRESSION((etree), type) ? ((Expression_##type *)(etree)->expression) \
                                : NULL)

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_ */