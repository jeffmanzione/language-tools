#include "lang/parser/parser.h"

#include <stdarg.h>

#include "alloc/alloc.h"
#include "struct/alist.h"

typedef struct _ParserBuilder {
  ProductionBuilder *root;
} ParserBuilder;

typedef enum { SINGLE, OR, AND } ProductionType;

typedef struct _ProductionBuilder {
  enum { PRODUCTION_SINGLE, PRODUCTION_OR, PRODUCTION_AND } type;
  AList children;
} ProductionBuilder;

ProductionBuilder *_production_builder_create(ProductionType type) {
  ProductionBuilder *pb = ALLOC2(ProductionBuilder);
  pb->type = type;
  alist_init(&pb->children, ProductionBuilder *, DEFAULT_ARRAY_SZ);
  return pb;
}

void production_builder_delete(ProductionBuilder *pb) {
  alist_finalize(&pb->children);
  DEALLOC(pb);
}

inline ProductionBuilder *
__production_builder_multi(ProductionType type, int arg_count, va_list valist) {
  ProductionBuilder *pb = _production_builder_create(type);
  for (i = 0; i < arg_count; i++) {
    ProductionBuilder *exp = va_arg(valist, Expression *);
    *alist_add(&pb->children) = exp;
  }
  va_end(valist);
}

ProductionBuilder *__or(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return __production_builder_multi(PRODUCTION_OR, arg_count, valist);
}

ProductionBuilder *__and(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return __production_builder_multi(PRODUCTION_AND, arg_count, valist);
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  pb->root = NULL;
  return pb;
}

void parser_builder_delete(ParserBuilder *pb) { DEALLOC(pb); }

void parser_builder_write_c_file(ParserBuilder *pb, FILE *file) {}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {}
