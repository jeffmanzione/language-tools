#ifndef LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_
#define LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_

#include "lang/parser/parser.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "struct/map.h"

typedef struct {
  Map populators;
  Map deleters;
} SemanticAnalyzer;

typedef void (*SemanticAnalyzerInitFn)(Map *, Map *);

void semantic_analyzer_init(SemanticAnalyzer *analyzer,
                            SemanticAnalyzerInitFn init_fn);
void semantic_analyzer_finalize(SemanticAnalyzer *analyzer);

ExpressionTree *semantic_analyzer_populate(SemanticAnalyzer *analyzer,
                                           const SyntaxTree *tree);
void semantic_analyzer_delete(SemanticAnalyzer *analyzer, ExpressionTree *tree);

#endif /* LANGUAGE_TOOLS_LANG_SEMANTIC_ANALYZER_SEMANTIC_ANALYZER_H_ */