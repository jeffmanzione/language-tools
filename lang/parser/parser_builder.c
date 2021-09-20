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
  default: // pass
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

const char *_create_rule_function_name(const char *production_name,
                                       char *suffix, int index) {
  char buffer[128];
  int len;
  if (index == 0) {
    len = sprintf(buffer, "rule_%s", production_name);
  } else {
    len =
        sprintf(buffer, "rule_%s__%s_helper%d", production_name, suffix, index);
  }
  return intern_range(buffer, 0, len);
}

void _write_rule_signature(const char *production_name, char *suffix, int index,
                           FILE *file) {
  fprintf(file, "SyntaxTree *");
  fprintf(file, "%s",
          _create_rule_function_name(production_name, suffix, index));
  fprintf(file, "(Parser *parser)");
}

const char *_suffix_for(const Production *p) {
  return PRODUCTION_TOKEN == p->type ? "token"
         : PRODUCTION_AND == p->type ? "and"
         : PRODUCTION_OR == p->type  ? "or"
                                     : NULL;
}

void _print_child_function_call(const char *production_name,
                                const Production *p, int index, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type ||
      PRODUCTION_TOKEN == p->type) {
    fprintf(file, "%s(parser);\n",
            _create_rule_function_name(production_name, _suffix_for(p), index));
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "&MATCH_EPSILON;\n");
  } else {
    ERROR("Unexpected production type: %d.", p->type);
  }
}

void _write_and_body(const char *production_name, const Production *p,
                     int index, FILE *file) {
  fprintf(file, "  SyntaxTree *st = parser_create_st(parser);\n");
  int child_index = index + 1;
  int next_index = index + alist_len(&p->children) + 1;
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    const Production *p_child = *(Production **)al_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");

    _print_child_function_call(production_name, p_child, child_index++, file);
    // SyntaxTree *st_child = _rule_or1__helper1(parser);

    fprintf(file, "    if (!st->matched) {\n");
    fprintf(file, "      parser_delete_st(parser, st);\n");
    fprintf(file, "      return &NO_MATCH;\n    }\n");
    fprintf(file, "    syntax_tree_add_child(st, st_child);\n");
    fprintf(file, "  }\n");
  }
  fprintf(file, "  return st;\n");
}

void _write_or_body(const char *production_name, const Production *p, int index,
                    FILE *file) {
  int child_index = index + 1;
  int next_index = index + alist_len(&p->children) + 1;
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    const Production *p_child = *(Production **)al_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");

    _print_child_function_call(production_name, p_child, child_index, file);
    // SyntaxTree *st_child = _rule_or1__helper1(parser);

    fprintf(file, "    if (st->matched) {\n      return st;\n    }\n  }\n");
  }
  fprintf(file, "  return &NO_MATCH;\n");
}

int _write_rule_and_subrules_helper(const char *production_name,
                                    const Production *p, int index,
                                    int next_index,
                                    TokenToStringFn token_to_str, FILE *file) {
  // printf("production_name=%s, index=%d\n", production_name, index);
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type) {
    int child_index = index + 1;
    int next_index = index + alist_len(&p->children) + 1;
    AL_iter children = alist_iter(&p->children);
    for (; al_has(&children); al_inc(&children)) {
      const Production *p_child = *(Production **)al_value(&children);
      if (PRODUCTION_EPSILON == p_child->type ||
          PRODUCTION_RULE == p_child->type) {
        continue;
      }
      next_index = _write_rule_and_subrules_helper(production_name, p_child,
                                                   child_index++, next_index,
                                                   token_to_str, file);
    }
  }
  _write_rule_signature(production_name, _suffix_for(p), index, file);

  fprintf(file, " {\n");
  if (PRODUCTION_AND == p->type) {
    _write_and_body(production_name, p, index, file);
  } else if (PRODUCTION_OR == p->type) {
    _write_or_body(production_name, p, index, file);
  } else if (PRODUCTION_TOKEN == p->type) {
    fprintf(file, "  Token *token = parser_next(parser);\n");
    fprintf(file, "  if (NULL == token || %s != token->type) {\n",
            token_to_str(p->token));
    fprintf(file, "    return &NO_MATCH;\n  }\n");
    fprintf(file, "  return match(parser);\n");
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "  return rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "  return &MATCH_EPSILON;\n");
  } else {
    ERROR("Unexpected production type: %d.", p->type);
  }
  fprintf(file, "}\n\n");
  return next_index;
}

void _write_rule_and_subrules(const char *production_name, const Production *p,
                              TokenToStringFn token_to_str, FILE *file) {
  _write_rule_and_subrules_helper(production_name, p, 0, 0, token_to_str, file);
}

void parser_builder_write_c_file(ParserBuilder *pb,
                                 TokenToStringFn token_to_str, FILE *file) {
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (Production *)value(&rules);
    _write_rule_and_subrules(production_name, p, token_to_str, file);
  }
}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    _write_rule_signature(production_name, NULL, 0, file);
    fprintf(file, ";\n");
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
