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
  // Production *root;
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

void _production_print(const Production *p, TokenToStringFn token_to_str,
                       FILE *out) {
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
  AL_iter iter = alist_iter(&p->children);
  fprintf(out, "%s(", PRODUCTION_AND == p->type ? "and" : "or");
  _production_print(*(Production **)al_value(&iter), token_to_str, out);
  al_inc(&iter);
  for (; al_has(&iter); al_inc(&iter)) {
    fprintf(out, ", ");
    _production_print(*(Production **)al_value(&iter), token_to_str, out);
  }
  fprintf(out, ")");
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = ALLOC2(ParserBuilder);
  // pb->root = NULL;
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

// void parser_builder_set_root(ParserBuilder *pb, Production *p) { pb->root =
// p; }

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
  if (!is_named_rule) {
    fprintf(file, "inline ");
  }
  fprintf(file, "SyntaxTree *");
  fprintf(file, "%s", _create_rule_function_name(production_name));
  fprintf(file, "(Parser *parser)");
}

const char *_suffix_for(const Production *p) {
  return PRODUCTION_TOKEN == p->type ? "token"
         : PRODUCTION_AND == p->type ? "and"
         : PRODUCTION_OR == p->type  ? "or"
                                     : NULL;
}

void _print_child_function_call(const char *production_name,
                                const Production *p, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type ||
      PRODUCTION_TOKEN == p->type) {
    fprintf(file, "%s(parser);\n",
            (char *)_create_rule_function_name(production_name));
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "&MATCH_EPSILON;\n");
  } else {
    ERROR("Unexpected production type: %d.", p->type);
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

void _write_and_body(const char *production_name, const Production *p,
                     FILE *file) {
  fprintf(file,
          "  SyntaxTree *st = parser_create_st(parser, rule_%s, \"%s\");\n",
          production_name, production_name);
  int child_index = -1;
  AL_iter children = alist_iter(&p->children);
  for (; al_has(&children); al_inc(&children)) {
    ++child_index;
    const Production *p_child = *(Production **)al_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");

    _print_child_function_call(_production_name_with_child_suffix(
                                   production_name, p_child, child_index),
                               p_child, file);

    fprintf(file, "    if (!st_child->matched) {\n");
    fprintf(file, "      parser_delete_st(parser, st);\n");
    fprintf(file, "      return &NO_MATCH;\n    }\n");
    fprintf(file, "    syntax_tree_add_child(st, st_child);\n");
    fprintf(file, "  }\n");
  }
  fprintf(file, "  st->matched = true;\n  return st;\n");
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
    fprintf(
        file,
        "    if (st_child->matched) {\n      return st_child;\n    }\n  }\n");
  }
  fprintf(file, "  return &NO_MATCH;\n");
}

void _write_rule_and_subrules(const char *production_name, const Production *p,
                              TokenToStringFn token_to_str, bool is_named_rule,
                              FILE *file) {
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
                               p_child, token_to_str, false, file);
    }
  }
  _write_rule_signature(production_name, p, is_named_rule, file);

  fprintf(file, " {\n");
  if (PRODUCTION_AND == p->type) {
    _write_and_body(production_name, p, file);
  } else if (PRODUCTION_OR == p->type) {
    _write_or_body(production_name, p, file);
  } else if (PRODUCTION_TOKEN == p->type) {
    fprintf(file, "  Token *token = parser_next(parser);\n");
    fprintf(file, "  if (NULL == token || %s != token->type) {\n",
            token_to_str(p->token));
    fprintf(file, "    return &NO_MATCH;\n  }\n");
    fprintf(file, "  return match(parser, rule_%s, \"%s\");\n", production_name,
            production_name);
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "  return rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "  return &MATCH_EPSILON;\n");
  } else {
    ERROR("Unexpected production type: %d.", p->type);
  }
  fprintf(file, "}\n\n");
}

void _write_header(ParserBuilder *pb, FILE *file, const char h_file_path[],
                   const char lexer_h_file_path[]) {
  // Includes.
  fprintf(file, "#include \"%s\"\n\n", h_file_path);
  fprintf(file, "#include \"lang/lexer/token.h\"\n");
  fprintf(file, "#include \"%s\"\n", lexer_h_file_path);
  fprintf(file, "\n");
}

void parser_builder_write_c_file(ParserBuilder *pb,
                                 TokenToStringFn token_to_str,
                                 const char h_file_path[],
                                 const char lexer_h_file_path[], FILE *file) {
  _write_header(pb, file, h_file_path, lexer_h_file_path);
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (Production *)value(&rules);
    _write_rule_and_subrules(production_name, p, token_to_str, true, file);
  }
}

void parser_builder_write_h_file(ParserBuilder *pb,
                                 TokenToStringFn token_to_str, FILE *file) {
  fprintf(file, "#ifndef LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_\n");
  fprintf(file, "#define LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_\n\n");
  fprintf(file, "#include \"lang/parser/parser.h\"\n");
  fprintf(file, "\n");
  M_iter rules = map_iter(&pb->rules);
  for (; has(&rules); inc(&rules)) {
    const char *production_name = (const char *)key(&rules);
    const Production *p = (const Production *)value(&rules);

    fprintf(file, "// %s -> ", production_name);
    _production_print(p, token_to_str, file);
    fprintf(file, ";\n");

    _write_rule_signature(production_name, p, true, file);
    fprintf(file, ";\n");
  }
  fprintf(file, "\n#endif /* LANGUAGE_TOOLS_LANG_PARSER_TODO_THIS_H_ */\n");
}

void parser_builder_print(ParserBuilder *pb, TokenToStringFn token_to_string,
                          FILE *out) {
  M_iter iter = map_iter(&pb->rules);
  for (; has(&iter); inc(&iter)) {
    fprintf(out, "%s -> ", (char *)key(&iter));
    _production_print((Production *)value(&iter), token_to_string, out);
    fprintf(out, ";\n");
  }
}
