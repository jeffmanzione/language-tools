#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_SA_SEMANTIC_ANALYZER_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_SA_SEMANTIC_ANALYZER_H_

#include "language-tools/parser/parser.h"
#include "language-tools/semantic_analyzer/expression_tree.h"

typedef struct {
  SAMap populators;
  SAMap producers;
  SAMap deleters;
} SemanticAnalyzer;

typedef ExpressionTree *(*Populator)(const SyntaxTree *tree,
                                     SemanticAnalyzer *analyzer);
typedef void (*EDeleter)(ExpressionTree *tree, SemanticAnalyzer *analyzer);

typedef void (*SemanticAnalyzerInitFn)(SAMap *, SAMap *, SAMap *);

void semantic_analyzer_init(SemanticAnalyzer *analyzer,
                            SemanticAnalyzerInitFn init_fn);
void semantic_analyzer_finalize(SemanticAnalyzer *analyzer);

ExpressionTree *semantic_analyzer_populate(SemanticAnalyzer *analyzer,
                                           const SyntaxTree *tree);

#define DEFINE_SEMANTIC_ANALYZER_PRODUCE_FN(ProduceType)              \
  typedef int (*Producer)(const ExpressionTree *tree,                 \
                          SemanticAnalyzer *analyzer, ProduceType *); \
  int semantic_analyzer_produce(SemanticAnalyzer *analyzer,           \
                                const ExpressionTree *tree,           \
                                ProduceType *target)

#define IMPL_SEMANTIC_ANALYZER_PRODUCE_FN(ProduceType)                        \
  int semantic_analyzer_produce(SemanticAnalyzer *analyzer,                   \
                                const ExpressionTree *tree,                   \
                                ProduceType *target) {                        \
    Producer produce = (Producer)SAMap_find(&analyzer->producers, tree->type, \
                                            sizeof(Producer), NULL);          \
    if (NULL == produce) {                                                    \
      fprintf(stderr, "Producer not found: %s", tree->rule_name);             \
      exit(1);                                                                \
    }                                                                         \
    return produce(tree, analyzer, target);                                   \
  }

void semantic_analyzer_delete(SemanticAnalyzer *analyzer, ExpressionTree *tree);

ExpressionTree *extract_tree_(ExpressionTreeArray *list_of_tree, int index);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_SA_SEMANTIC_ANALYZER_H_  */