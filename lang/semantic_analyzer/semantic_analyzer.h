#ifndef LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_
#define LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_

#include "lang/parser/parser.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "struct/map.h"

typedef struct {
  Map populators;
  Map producers;
  Map deleters;
} SemanticAnalyzer;

typedef ExpressionTree *(*Populator)(const SyntaxTree *tree,
                                     SemanticAnalyzer *analyzer);
typedef void (*EDeleter)(ExpressionTree *tree, SemanticAnalyzer *analyzer);

typedef void (*SemanticAnalyzerInitFn)(Map *, Map *, Map *);

void semantic_analyzer_init(SemanticAnalyzer *analyzer,
                            SemanticAnalyzerInitFn init_fn);
void semantic_analyzer_finalize(SemanticAnalyzer *analyzer);

ExpressionTree *semantic_analyzer_populate(SemanticAnalyzer *analyzer,
                                           const SyntaxTree *tree);

#define DEFINE_SEMANTIC_ANALYZER_PRODUCE_FN(ProduceType)                       \
  typedef int (*Producer)(const ExpressionTree *tree,                          \
                          SemanticAnalyzer *analyzer, ProduceType *);          \
  int semantic_analyzer_produce(SemanticAnalyzer *analyzer,                    \
                                const ExpressionTree *tree,                    \
                                ProduceType *target)

#define IMPL_SEMANTIC_ANALYZER_PRODUCE_FN(ProduceType)                         \
  int semantic_analyzer_produce(SemanticAnalyzer *analyzer,                    \
                                const ExpressionTree *tree,                    \
                                ProduceType *target) {                         \
    ASSERT(NOT_NULL(analyzer));                                                \
    ASSERT(NOT_NULL(tree));                                                    \
    ASSERT(NOT_NULL(target));                                                  \
    Producer produce = (Producer)map_lookup(&analyzer->producers, tree->type); \
    if (NULL == produce) {                                                     \
      FATALF("Producer not found: %s", tree->rule_name);                       \
    }                                                                          \
    return produce(tree, analyzer, target);                                    \
  }

void semantic_analyzer_delete(SemanticAnalyzer *analyzer, ExpressionTree *tree);

ExpressionTree *__extract_tree(AList *alist_of_tree, int index);

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_ */