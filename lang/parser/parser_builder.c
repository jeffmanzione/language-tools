#include <stdarg.h>

#include "alloc/alloc.h"
#include "struct/alist.h"

// Hardcoded in lexer.
#define TOKEN_NEWLINE 1

typedef enum { SINGLE, OR, AND } ProductionType;

typedef struct _ProductionBuilder {
  enum {
    PRODUCTION_EPSILON,
    PRODUCTION_TOKEN,
    PRODUCTION_OR,
    PRODUCTION_AND
  } type;
  union {
    AList children;
    int token;
  };
} ProductionBuilder;

typedef struct _ParserBuilder {
  ProductionBuilder *root;
} ParserBuilder;

ProductionBuilder *_production_builder_create(ProductionType type) {
  ProductionBuilder *pb = ALLOC2(ProductionBuilder);
  pb->type = type;
  return pb;
}

void production_builder_delete(ProductionBuilder *pb) {
  alist_finalize(&pb->children);
  DEALLOC(pb);
}

inline ProductionBuilder *_production_builder_multi(ProductionType type,
                                                    int arg_count,
                                                    va_list valist) {
  ProductionBuilder *pb = _production_builder_create(type);
  alist_init(&pb->children, ProductionBuilder *, DEFAULT_ARRAY_SZ);
  int i;
  for (i = 0; i < arg_count; i++) {
    ProductionBuilder *exp = va_arg(valist, ProductionBuilder *);
    alist_append(&pb->children, &exp);
  }
  va_end(valist);
  return pb;
}

ProductionBuilder *__or(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return _production_builder_multi(PRODUCTION_OR, arg_count, valist);
}

ProductionBuilder *__and(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return _production_builder_multi(PRODUCTION_AND, arg_count, valist);
}

ProductionBuilder *token(int token) {
  ProductionBuilder *pb = _production_builder_create(PRODUCTION_TOKEN);
  pb->token = token;
  return pb;
}

ProductionBuilder *newline() { return token(TOKEN_NEWLINE); }

ProductionBuilder *line(ProductionBuilder *pb) {
  ProductionBuilder *pb_parent =
      _production_builder_multi(PRODUCTION_AND, 0, NULL);
  alist_append(&pb_parent->children, &pb);
  ProductionBuilder *nl = newline();
  alist_append(&pb_parent->children, &nl);
  return pb_parent;
}

ProductionBuilder *epsilon() {
  return _production_builder_create(PRODUCTION_EPSILON);
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  pb->root = NULL;
  return pb;
}

void parser_builder_delete(ParserBuilder *pb) { DEALLOC(pb); }

void parser_builder_write_c_file(ParserBuilder *pb, FILE *file) {}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {}
