#include "language-tools/semantic_analyzer/semantic_analyzer.h"

ExpressionTree *extract_tree_(ExpressionTreeArray *alist_of_tree, int index) {
  return ExpressionTreeArray_get_unchecked(alist_of_tree, index);
}

void semantic_analyzer_init(SemanticAnalyzer *analyzer,
                            SemanticAnalyzerInitFn init_fn) {
  SAMap_init(&analyzer->populators, SAMap_ptr_hasher, SAMap_ptr_comparator);
  SAMap_init(&analyzer->producers, SAMap_ptr_hasher, SAMap_ptr_comparator);
  SAMap_init(&analyzer->deleters, SAMap_ptr_hasher, SAMap_ptr_comparator);
  init_fn(&analyzer->populators, &analyzer->producers, &analyzer->deleters);
}

void semantic_analyzer_finalize(SemanticAnalyzer *analyzer) {
  SAMap_finalize(&analyzer->populators);
  SAMap_finalize(&analyzer->producers);
  SAMap_finalize(&analyzer->deleters);
}

ExpressionTree *semantic_analyzer_populate(SemanticAnalyzer *analyzer,
                                           const SyntaxTree *tree) {
  Populator populate = (Populator)SAMap_find(
      &analyzer->populators, tree->rule_fn, sizeof(Populator), NULL);
  if (NULL == populate) {
    fprintf(stderr, "Populator not found: %s", tree->production_name);
    exit(1);
  }
  return populate(tree, analyzer);
}

void semantic_analyzer_delete(SemanticAnalyzer *analyzer,
                              ExpressionTree *tree) {
  EDeleter del = (EDeleter)SAMap_find(&analyzer->deleters, tree->type,
                                      sizeof(EDeleter), NULL);
  if (NULL == del) {
    fprintf(stderr, "Deleter not found: %s", tree->rule_name);
    exit(1);
  }
  del(tree, analyzer);
  free(tree->expression);
  free(tree);
}