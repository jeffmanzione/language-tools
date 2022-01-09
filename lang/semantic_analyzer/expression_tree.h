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

#define _GET_MACRO(_1, _2, NAME) NAME
#define DEFINE_EXPRESSION(...)                                                 \
  _GET_MACRO(__VA_ARGS__, DEFINE_EXPRESSION_WITH_PRODUCER,                     \
             DEFINE_EXPRESSION_NO_PRODUCER)                                    \
  (__VA_ARGS__)

#define DEFINE_EXPRESSION_NO_PRODUCER(name)                                    \
  typedef struct _Expression_##name Expression_##name;                         \
  ExpressionTree *Populate_##name(const SyntaxTree *tree,                      \
                                  SemanticAnalyzer *analyzer);                 \
  void Transform_##name(const SyntaxTree *tree, Expression_##name *name,       \
                        SemanticAnalyzer *analyzer);                           \
  void Delete_##name(ExpressionTree *tree, SemanticAnalyzer *analyzer);        \
  void Delete_##name##_inner(Expression_##name *name,                          \
                             SemanticAnalyzer *analyzer);                      \
  struct _Expression_##name

#define DEFINE_EXPRESSION_WITH_PRODUCER(name, ProduceType)                     \
  typedef struct _Expression_##name Expression_##name;                         \
  ExpressionTree *Populate_##name(const SyntaxTree *tree,                      \
                                  SemanticAnalyzer *analyzer);                 \
  void Transform_##name(const SyntaxTree *tree, Expression_##name *name,       \
                        SemanticAnalyzer *analyzer);                           \
  void Delete_##name(ExpressionTree *tree, SemanticAnalyzer *analyzer);        \
  void Delete_##name##_inner(Expression_##name *name,                          \
                             SemanticAnalyzer *analyzer);                      \
  int Produce_##name(ExpressionTree *tree, SemanticAnalyzer *analyzer,         \
                     ProduceType *);                                           \
  int Produce_##name##_inner(Expression_##name *name,                          \
                             SemanticAnalyzer *analyzer, ProduceType *);       \
  struct _Expression_##name

#define POPULATE_IMPL(name, stree_input, analyzer_input)                       \
  ExpressionTree *Populate_##name(stree_input, analyzer_input) {               \
    ExpressionTree *etree = ALLOC2(ExpressionTree);                            \
    etree->type = rule_##name;                                                 \
    etree->rule_name = #name;                                                  \
    etree->expression = ALLOC(Expression_##name);                              \
    Transform_##name(stree, etree->expression, analyzer);                      \
    return etree;                                                              \
  }                                                                            \
  void Transform_##name(stree_input, Expression_##name *name, analyzer_input)

#define PRODUCE_IMPL(name, analyzer_input, producer_input)                     \
  int Produce_##name(ExpressionTree *tree, analyzer_input, producer_input) {   \
    return Produce_##name##_inner((Expression_##name *)tree->expression,       \
                                  analyzer, target);                           \
  }                                                                            \
  int Produce_##name##_inner(Expression_##name *name, analyzer_input,          \
                             producer_input)

#define DELETE_IMPL(name, analyzer_input)                                      \
  void Delete_##name(ExpressionTree *tree, analyzer_input) {                   \
    Delete_##name##_inner((Expression_##name *)tree->expression, analyzer);    \
  }                                                                            \
  void Delete_##name##_inner(Expression_##name *name, analyzer_input)

#define REGISTRATION_FN(name)                                                  \
  void name(Map *populators, Map *producers, Map *deleters)

#define REGISTER_EXPRESSION(name)                                              \
  {                                                                            \
    map_insert(populators, rule_##name, Populate_##name);                      \
    map_insert(deleters, rule_##name, Delete_##name);                          \
  }

#define REGISTER_EXPRESSION_WITH_PRODUCER(name)                                \
  {                                                                            \
    map_insert(populators, rule_##name, Populate_##name);                      \
    map_insert(producers, rule_##name, Produce_##name);                        \
    map_insert(deleters, rule_##name, Delete_##name);                          \
  }

#define EXPECT_TYPE(stree, type)                                               \
  if (stree->rule_fn != type) {                                                \
    FATALF("Expected type: " #type);                                           \
  }

#define IS_SYNTAX(stree, type) (((stree)->rule_fn) == (type))

#define HAS_TOKEN(stree) ((NULL != (stree)) && (NULL != (stree->token)))

#define IS_TOKEN(stree, token_type)                                            \
  (HAS_TOKEN(stree) && ((stree)->token->type == (token_type)))

#define TOKEN_TEXT_FOR(stree)                                                  \
  (((NULL != (stree)) && (NULL != (stree->token))) ? stree->token->text : NULL)

#define CHILD_SYNTAX_AT(stree, index)                                          \
  (alist_len(&(stree)->children) > (index)                                     \
       ? *(SyntaxTree **)alist_get(&(stree)->children, (index))                \
       : NULL)

#define CHILD_IS_SYNTAX(stree, index, type)                                    \
  (IS_SYNTAX(CHILD_SYNTAX_AT(stree, (index)), (type)))

#define CHILD_HAS_TOKEN(stree, index) HAS_TOKEN(CHILD_SYNTAX_AT(stree, (index)))

#define CHILD_IS_TOKEN(stree, index, token_type)                               \
  IS_TOKEN(CHILD_SYNTAX_AT((stree), (index)), (token_type))

#define CHILD_COUNT(stree) (alist_len(&(stree)->children))

#define IS_EXPRESSION(etree, typ)                                              \
  ((NULL != (etree)) && ((rule_##typ) == (etree)->type))

#define EXTRACT_EXPRESSION(etree, type)                                        \
  (IS_EXPRESSION((etree), type) ? ((Expression_##type *)(etree)->expression)   \
                                : NULL)

#define APPEND_TREE(sa, alist_of_tree, stree)                                  \
  {                                                                            \
    ExpressionTree *expr = semantic_analyzer_populate(sa, stree);              \
    alist_append(alist_of_tree, (void *)&expr);                                \
  }

#define EXTRACT_TREE(alist_of_tree, i) __extract_tree(alist_of_tree, i)

#define DECLARE_IF_TYPE(name, type, stree)                                     \
  SyntaxTree *name;                                                            \
  {                                                                            \
    if (stree->rule_fn != type) {                                              \
      FATALF("Expected " #type " for " #name);                                 \
      name = NULL;                                                             \
    } else {                                                                   \
      name = stree;                                                            \
    }                                                                          \
  }

#define ASSIGN_IF_TYPE(name, type, stree)                                      \
  {                                                                            \
    if (stree->rule_fn != type) {                                              \
      FATALF("Expected " #type " for " #name);                                 \
    } else {                                                                   \
      name = stree;                                                            \
    }                                                                          \
  }

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_EXPRESSION_TREE_H_ */