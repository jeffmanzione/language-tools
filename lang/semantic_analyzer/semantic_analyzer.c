#include "lang/semantic_analyzer/semantic_analyzer.h"

#include "debug/debug.h"

ExpressionTree *__extract_tree(AList *alist_of_tree, int index) {
  ExpressionTree **tree_ptr2 =
      (ExpressionTree **)alist_get(alist_of_tree, index);
  return *tree_ptr2;
}

void semantic_analyzer_init(SemanticAnalyzer *analyzer,
                            SemanticAnalyzerInitFn init_fn) {
  map_init_default(&analyzer->populators);
  map_init_default(&analyzer->producers);
  map_init_default(&analyzer->deleters);
  init_fn(&analyzer->populators, &analyzer->producers, &analyzer->deleters);
}

void semantic_analyzer_finalize(SemanticAnalyzer *analyzer) {
  map_finalize(&analyzer->populators);
  map_finalize(&analyzer->producers);
  map_finalize(&analyzer->deleters);
}

ExpressionTree *semantic_analyzer_populate(SemanticAnalyzer *analyzer,
                                           const SyntaxTree *tree) {
  Populator populate =
      (Populator)map_lookup(&analyzer->populators, tree->rule_fn);
  if (NULL == populate) {
    ERROR("Populator not found: %s", tree->production_name);
  }
  return populate(tree, analyzer);
}

void semantic_analyzer_delete(SemanticAnalyzer *analyzer,
                              ExpressionTree *tree) {
  EDeleter delete = (EDeleter)map_lookup(&analyzer->deleters, tree->type);
  if (NULL == delete) {
    ERROR("Deleter not found: %s", tree->rule_name);
  }
  delete (tree, analyzer);
  DEALLOC(tree->expression);
  DEALLOC(tree);
}