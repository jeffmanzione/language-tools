#include "lang/parser/parser_builder.h"

#include <stdarg.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "debug/debug.h"
#include "struct/alist.h"
#include "struct/map.h"
#include "struct/struct_defaults.h"
#include "util/string.h"

// Hardcoded in lexer.
#define TOKEN_NEWLINE 1

typedef enum {
  PRODUCTION_EPSILON,
  PRODUCTION_TOKEN,
  PRODUCTION_OR,
  PRODUCTION_AND,
  PRODUCTION_RULE,
  PRODUCTION_OPTIONAL
} ProductionType;

typedef struct _Production {
  ProductionType type;
  union {
    AList children;
    const char *token;
    char *rule_name;
  };
} Production;

typedef struct _ParserBuilder {
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

Production *_production_multi_helper(ProductionType type) {
  Production *p = _production_create(type);
  alist_init(&p->children, Production *, DEFAULT_ARRAY_SZ);
  return p;
}

Production *production_and() {
  return _production_multi_helper(PRODUCTION_AND);
}

Production *production_or() { return _production_multi_helper(PRODUCTION_OR); }

void production_add_child(Production *parent, Production *child) {
  alist_append(&parent->children, &child);
}

Production *_production_multi(ProductionType type, int arg_count,
                              va_list valist) {
  Production *p = _production_multi_helper(type);
  int i;
  for (i = 0; i < arg_count; i++) {
    Production *exp = va_arg(valist, Production *);
    alist_append(&p->children, &exp);
  }
  va_end(valist);
  return p;
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

Production *token(const char token[]) {
  Production *p = _production_create(PRODUCTION_TOKEN);
  p->token = token;
  return p;
}

Production *optional(Production *p_child) {
  Production *p = _production_create(PRODUCTION_OPTIONAL);
  alist_init(&p->children, Production *, DEFAULT_ARRAY_SZ);
  alist_append(&p->children, &p_child);
  return p;
}

Production *newline() { return token("TOKEN_NEWLINE"); }

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

void _production_print(const Production *p, FILE *out) {
  switch (p->type) {
  case PRODUCTION_EPSILON:
    fprintf(out, "E");
    return;
  case PRODUCTION_RULE:
    fprintf(out, "rule:%s", p->rule_name);
    return;
  case PRODUCTION_TOKEN:
    fprintf(out, "token:%s", p->token);
    return;
  default: // pass
    break;
  }
  // Must be AND or OR.
  AL_iter iter = alist_iter(&p->children);
  const char *production_type = PRODUCTION_AND == p->type  ? "AND"
                                : PRODUCTION_OR == p->type ? "OR"
                                                           : "OPTIONAL";
  fprintf(out, "%s(", production_type);
  _production_print(*(Production **)al_value(&iter), out);
  al_inc(&iter);
  for (; al_has(&iter); al_inc(&iter)) {
    fprintf(out, ", ");
    _production_print(*(Production **)al_value(&iter), out);
  }
  fprintf(out, ")");
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  map_init_default(&pb->rules);
  return pb;
}

void parser_builder_rule(ParserBuilder *pb, const char rule_name[],
                         Production *p) {
  const char *interned_rule_name = intern(rule_name);
  if (NULL != map_lookup(&pb->rules, interned_rule_name)) {
    FATALF("Multiple rules with name '%s'.", rule_name);
  }
  map_insert(&pb->rules, interned_rule_name, p);
}

void parser_builder_delete(ParserBuilder *pb) {
  M_iter iter = map_iter(&pb->rules);
  for (; has(&iter); inc(&iter)) {
    _production_delete(value(&iter));
  }
  map_finalize(&pb->rules);
  DEALLOC(pb);
}

const char *_create_rule_function_name(const char *production_name) {
  char buffer[128];
  int len = sprintf(buffer, "rule_%s", production_name);
  return intern_range(buffer, 0, len);
}

void _write_rule_signature(const char *production_name, const Production *p,
                           bool is_named_rule, FILE *file) {
  // if (!is_named_rule) {
  //   fprintf(file, "inline ");
  // }
  fprintf(file, "SyntaxTree *");
  fprintf(file, "%s", _create_rule_function_name(production_name));
  fprintf(file, "(Parser *parser)");
}

const char *_suffix_for(const Production *p) {
  return PRODUCTION_TOKEN == p->type      ? "token"
         : PRODUCTION_AND == p->type      ? "and"
         : PRODUCTION_OR == p->type       ? "or"
         : PRODUCTION_OPTIONAL == p->type ? "opt"
                                          : NULL;
}

void _print_child_function_call(const char *production_name,
                                const Production *p, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type ||
      PRODUCTION_TOKEN == p->type || PRODUCTION_OPTIONAL == p->type) {
    fprintf(file, "%s(parser);\n",
            (char *)_create_rule_function_name(production_name));
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "&MATCH_EPSILON;\n");
  } else {
    FATALF("Unexpected production type: %d.", p->type);
  }
}

const char *_production_name_with_child_suffix(const char *production_name,
                                               const Production *p,
                                               int child_index) {
  char buffer[128];
  int len =
      sprintf(buffer, "%s__%s%d", production_name, _suffix_for(p), child_index);
  return intern_range(buffer, 0, len);
}

bool _is_helper_rule(const char production_name[]) {
  return NULL !=
         find_str((char *)production_name, strlen(production_name), "__", 2);
}

void _write_and_body(const char *production_name, const Production *p,
                     FILE *file) {
  if (_is_helper_rule(production_name)) {
    fprintf(file, "  SyntaxTree *st = parser_create_st(parser, NULL, \"\");\n");
  } else {
    fprintf(file,
            "  SyntaxTree *st = parser_create_st(parser, rule_%s, \"%s\");\n",
            production_name, production_name);
  }
  int child_index = -1;
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    ++child_index;
    const Production *p_child = *(Production **)al_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");

    if (PRODUCTION_OPTIONAL == p_child->type) {
      _print_child_function_call(_production_name_with_child_suffix(
                                     production_name, p_child, child_index),
                                 p_child, file);
      fprintf(file, "    if (st_child->matched) {\n");
      fprintf(file,
              "       syntax_tree_add_child(st, st_child);\n    }\n  }\n");
    } else {
      _print_child_function_call(_production_name_with_child_suffix(
                                     production_name, p_child, child_index),
                                 p_child, file);
      fprintf(file, "    if (!st_child->matched) {\n");
      fprintf(file, "      parser_delete_st(parser, st);\n");
      fprintf(file, "      return &NO_MATCH;\n    }\n");
      fprintf(file, "    syntax_tree_add_child(st, st_child);\n  }\n");
    }
  }
  fprintf(file, "  if (!st->has_children) {\n");
  fprintf(file, "    parser_delete_st(parser, st);\n");
  fprintf(file, "    return &NO_MATCH;\n  }\n");
  fprintf(file,
          "  st->matched = true;\n  return parser_prune_st(parser, st);\n");
}

void _write_or_body(const char *production_name, const Production *p,
                    FILE *file) {
  int child_index = -1;
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    ++child_index;
    const Production *p_child = *(Production **)al_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");
    _print_child_function_call(_production_name_with_child_suffix(
                                   production_name, p_child, child_index),
                               p_child, file);
    fprintf(file, "    if (st_child->matched) {\n");
    fprintf(file, "      if (NULL == st_child->rule_fn) {\n");
    fprintf(file, "        st_child->rule_fn = rule_%s;\n", production_name);
    fprintf(file, "        st_child->production_name = \"%s\";\n",
            production_name);
    fprintf(file, "      }\n");
    fprintf(file, "      return st_child;\n    }\n  }\n");
  }
  fprintf(file, "  return &NO_MATCH;\n");
}

void _write_rule_and_subrules(const char *production_name, const Production *p,
                              bool is_named_rule, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type) {
    int child_index = -1;
    AL_iter children = alist_iter(&p->children);
    for (; al_has(&children); al_inc(&children)) {
      ++child_index;
      const Production *p_child = *(Production **)al_value(&children);
      if (PRODUCTION_EPSILON == p_child->type ||
          PRODUCTION_RULE == p_child->type) {
        continue;
      }
      _write_rule_and_subrules(_production_name_with_child_suffix(
                                   production_name, p_child, child_index),
                               p_child, false, file);
    }
  }
  if (PRODUCTION_OPTIONAL == p->type) {
    p = *(Production **)alist_get(&p->children, 0);
    _write_rule_and_subrules(production_name, p, is_named_rule, file);
    return;
  }
  _write_rule_signature(production_name, p, is_named_rule, file);

  fprintf(file, " {\n");
  if (PRODUCTION_AND == p->type) {
    _write_and_body(production_name, p, file);
  } else if (PRODUCTION_OR == p->type) {
    _write_or_body(production_name, p, file);
  } else if (PRODUCTION_TOKEN == p->type) {
    fprintf(file, "  Token *token = parser_next(parser);\n");
    fprintf(file, "  if (NULL == token || %s != token->type) {\n", p->token);
    fprintf(file, "    return &NO_MATCH;\n  }\n");
    if (0 == strncmp("__token",
                     production_name + strlen(production_name) -
                         strlen("__token") - 1,
                     strlen("__token"))) {
      fprintf(file, "  return match(parser, NULL, NULL);\n");
    } else {
      fprintf(file, "  return match(parser, rule_%s, \"%s\");\n",
              production_name, production_name);
    }
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "  return rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "  return &MATCH_EPSILON;\n");
  } else {
    FATALF("Unexpected production type: %d.", p->type);
  }
  fprintf(file, "}\n\n");
}

void _write_headers(ParserBuilder *pb, FILE *file, const char h_file_path[],
                    const char lexer_h_file_path[]) {
  // Includes.
  fprintf(file, "#include \"%s\"\n\n", h_file_path);
  fprintf(file, "#include \"lang/lexer/token.h\"\n");
  fprintf(file, "#include \"%s\"\n", lexer_h_file_path);
  fprintf(file, "\n");
}

void parser_builder_write_c_file(ParserBuilder *pb, const char h_file_path[],
                                 const char lexer_h_file_path[], FILE *file) {
  _write_headers(pb, file, h_file_path, lexer_h_file_path);
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (Production *)value(&rules);
    _write_rule_and_subrules(production_name, p, true, file);
  }
}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {
  fprintf(file, "#ifndef LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_\n");
  fprintf(file, "#define LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_\n\n");
  fprintf(file, "#include \"lang/parser/parser.h\"\n");
  fprintf(file, "\n");
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (const Production *)value(&rules);

    fprintf(file, "// %s -> ", production_name);
    _production_print(p, file);
    fprintf(file, ";\n");

    _write_rule_signature(production_name, p, true, file);
    fprintf(file, ";\n");
  }
  fprintf(file, "\n#endif /* LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_ */\n");
}

void parser_builder_print(ParserBuilder *pb, FILE *out) {
  M_iter iter = map_iter(&pb->rules);
  for (; has(&iter); inc(&iter)) {
    fprintf(out, "%s -> ", (char *)key(&iter));
    _production_print((Production *)value(&iter), out);
    fprintf(out, ";\n");
  }
}
