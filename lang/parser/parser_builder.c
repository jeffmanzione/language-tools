#include <stdarg.h>

#include "alloc/alloc.h"
#include "struct/alist.h"

// Hardcoded in lexer.
#define TOKEN_NEWLINE 1

typedef enum { SINGLE, OR, AND } ProductionType;

typedef struct _Production {
  char *name;
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
} Production;

typedef struct _ParserBuilder {
  Production *root;
} ParserBuilder;

Production *_production_create(ProductionType type) {
  Production *pb = ALLOC2(Production);
  pb->type = type;
  return pb;
}

void _production_delete(Production *p) {
  if (p->type == PRODUCTION_OR || p->type == PRODUCTION_AND) {
    AL_iter iter = alist_iter(&p->children);
    for (; al_has(&iter); al_inc(&iter)) {
      _production_delete(*((Production **)al_value(&iter)));
    }
    alist_finalize(&p->children);
  }
  DEALLOC(p);
}

inline Production *_production_multi(ProductionType type, int arg_count,
                                     va_list valist) {
  Production *pb = _production_create(type);
  alist_init(&pb->children, Production *, DEFAULT_ARRAY_SZ);
  int i;
  for (i = 0; i < arg_count; i++) {
    Production *exp = va_arg(valist, Production *);
    alist_append(&pb->children, &exp);
  }
  va_end(valist);
  return pb;
}

Production *__or(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return _production_multi(PRODUCTION_OR, arg_count, valist);
}

Production *__and(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return _production_multi(PRODUCTION_AND, arg_count, valist);
}

Production *token(int token) {
  Production *pb = _production_create(PRODUCTION_TOKEN);
  pb->token = token;
  return pb;
}

Production *newline() { return token(TOKEN_NEWLINE); }

Production *line(Production *pb) {
  Production *pb_parent = _production_multi(PRODUCTION_AND, 0, NULL);
  alist_append(&pb_parent->children, &pb);
  Production *nl = newline();
  alist_append(&pb_parent->children, &nl);
  return pb_parent;
}

Production *epsilon() { return _production_create(PRODUCTION_EPSILON); }

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  pb->root = NULL;
  return pb;
}

void parser_builder_set_root(ParserBuilder *pb, Production *p) { pb->root = p; }

void parser_builder_delete(ParserBuilder *pb) {
  if (NULL != pb->root) {
    _production_delete(pb->root);
  }
  DEALLOC(pb);
}

void parser_builder_write_c_file(ParserBuilder *pb, FILE *file) {}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {}
