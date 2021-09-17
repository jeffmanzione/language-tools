#include "lang/parser/parser_builder.h"

#include <stdarg.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "debug/debug.h"
#include "struct/alist.h"
#include "struct/map.h"

// Hardcoded in lexer.
#define TOKEN_NEWLINE 1

typedef enum { SINGLE, OR, AND } ProductionType;

typedef struct _Production {
  enum {
    PRODUCTION_EPSILON,
    PRODUCTION_TOKEN,
    PRODUCTION_OR,
    PRODUCTION_AND,
    PRODUCTION_RULE
  } type;
  union {
    AList children;
    int token;
    char *rule_name;
  };
} Production;

typedef struct _ParserBuilder {
  Production *root;
  Map rules;
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

Production *line(Production *p) {
  Production *p_parent = _production_multi(PRODUCTION_AND, 0, NULL);
  alist_append(&p_parent->children, &p);
  Production *nl = newline();
  alist_append(&p_parent->children, &nl);
  return p_parent;
}

Production *epsilon() { return _production_create(PRODUCTION_EPSILON); }

Production *rule(const char rule_name[]) {
  Production *p = _production_create(PRODUCTION_RULE);
  p->rule_name = intern(rule_name);
  return p;
}

void _production_print(Production *p, TokenToStringFn token_to_str, FILE *out) {
  switch (p->type) {
    case PRODUCTION_EPSILON:
      fprintf(out, "E");
      return;
    case PRODUCTION_RULE:
      fprintf(out, "rule:%s", p->rule_name);
      return;
    case PRODUCTION_TOKEN:
      fprintf(out, "token:%s", token_to_str(p->token));
      return;
    default:  // pass
      break;
  }
  // Must be AND or OR.
  const char *sep_word = PRODUCTION_AND == p->type ? " " : " | ";
  AL_iter iter = alist_iter(&p->children);
  if (p->type == PRODUCTION_OR) {
    fprintf(out, "( ");
  }
  _production_print(*(Production **)al_value(&iter), token_to_str, out);
  al_inc(&iter);
  for (; al_has(&iter); al_inc(&iter)) {
    fprintf(out, "%s", sep_word);
    _production_print(*(Production **)al_value(&iter), token_to_str, out);
  }
  if (p->type == PRODUCTION_OR) {
    fprintf(out, " )");
  }
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  pb->root = NULL;
  map_init_default(&pb->rules);
  return pb;
}

void parser_builder_rule(ParserBuilder *pb, const char rule_name[],
                         Production *p) {
  const char *interned_rule_name = intern(rule_name);
  if (NULL != map_lookup(&pb->rules, interned_rule_name)) {
    ERROR("Multiple rules with name '%s'.", rule_name);
  }
  map_insert(&pb->rules, interned_rule_name, p);
}

void parser_builder_set_root(ParserBuilder *pb, Production *p) { pb->root = p; }

void parser_builder_delete(ParserBuilder *pb) {
  M_iter iter = map_iter(&pb->rules);
  for (; has(&iter); inc(&iter)) {
    _production_delete(value(&iter));
  }
  map_finalize(&pb->rules);
  DEALLOC(pb);
}

void _write_rule_signature(const char *production_name, FILE *file) {
  fprintf(file, "SyntaxTree *rule_%s(Parser *parser)", production_name);
}

void _write_rule_and_subrule_signatures(const char *production_name,
                                        const Production *p, FILE *file) {
  _write_rule_signature(production_name, file);
  fprintf(file, ";\n");
  if (PRODUCTION_EPSILON == p->type || PRODUCTION_TOKEN == p->type ||
      PRODUCTION_TOKEN == p->type) {
    return;
  }
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    const Production *p_child = (Production *)al_value(&children);
  }
}

void parser_builder_write_c_file(ParserBuilder *pb, FILE *file) {}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (Production *)value(&rules);
    _write_rule_and_subrule_signatures(production_name, p, file);
  }
}

void parser_builder_print(ParserBuilder *pb, TokenToStringFn token_to_string,
                          FILE *out) {
  M_iter iter = map_iter(&pb->rules);
  for (; has(&iter); inc(&iter)) {
    fprintf(out, "%s -> ", (char *)key(&iter));
    _production_print((Production *)value(&iter), token_to_string, out);
    fprintf(out, "\n");
  }
}
