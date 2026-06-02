#include "language-tools/semantic_analyzer/expression_tree.h"

IMPL_ARRAYLIKE(ExpressionTreeArray, ExpressionTree *);
IMPL_MAPLIKE(SAMap, void *, void *);

uint32_t SAMap_ptr_hasher(const void *ptr, uint32_t size) {
  return (uint32_t)(intptr_t)ptr;
}

int32_t SAMap_ptr_comparator(const void *ptr1, uint32_t ptr1_len,
                             const void *ptr2, uint32_t ptr2_len) {
  return ((intptr_t)ptr1) - ((intptr_t)ptr2);
}