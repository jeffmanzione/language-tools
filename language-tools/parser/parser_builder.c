#include "language-tools/parser/parser_builder.h"

#include <stdarg.h>

#include "c-data-structures/arraylike.h"
#include "c-data-structures/maplike.h"
#include "file-utils/string_utils.h"
#include "language-tools/intern.h"

DEFINE_ARRAYLIKE(ProductionArray, Production *);
IMPL_ARRAYLIKE(ProductionArray, Production *);

DEFINE_MAPLIKE(ProductionMap, char *, Production *);
IMPL_MAPLIKE(ProductionMap, char *, Production *);

uint32_t string_ptr_hasher_(const char *ptr, uint32_t size) {
  return (uint32_t)(intptr_t)ptr;
}

int32_t string_ptr_comparator_(const char *ptr1, uint32_t ptr1_len,
                               const char *ptr2, uint32_t ptr2_len) {
  return ((intptr_t)ptr1) - ((intptr_t)ptr2);
}

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

typedef struct Production_ {
  bool exclude_from_header;
  ProductionType type;
  union {
    ProductionArray children;
    const char *token;
    const char *rule_name;
  };
} Production;

typedef struct ParserBuilder_ {
  ProductionMap rules;
} ParserBuilder;

Production *production_create_(ProductionType type) {
  Production *p = malloc(sizeof(Production));
  p->type = type;
  p->exclude_from_header = false;
  return p;
}

void production_delete_(Production *p) {
  if (p->type == PRODUCTION_OR || p->type == PRODUCTION_AND) {
    ProductionArrayIterator iter;
    ProductionArray_iterator(&iter, &p->children);
    for (; ProductionArray_has_next(&iter); ProductionArray_next(&iter)) {
      production_delete_(*ProductionArray_mutable_value(&iter));
    }
    ProductionArray_finalize(&p->children);
  }
  free(p);
}

Production *production_multi_helper_(ProductionType type) {
  Production *p = production_create_(type);
  ProductionArray_init(&p->children);
  return p;
}

Production *production_and() {
  return production_multi_helper_(PRODUCTION_AND);
}

Production *production_or() { return production_multi_helper_(PRODUCTION_OR); }

void production_add_child(Production *parent, Production *child) {
  ProductionArray_push_back(&parent->children, child);
}

void production_exclude_from_header(Production *p) {
  p->exclude_from_header = true;
}

Production *production_multi_(ProductionType type, int arg_count,
                              va_list valist) {
  Production *p = production_multi_helper_(type);
  int i;
  for (i = 0; i < arg_count; i++) {
    Production *exp = va_arg(valist, Production *);
    ProductionArray_push_back(&p->children, exp);
  }
  va_end(valist);
  return p;
}

Production *or__(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return production_multi_(PRODUCTION_OR, arg_count, valist);
}

Production *and__(int arg_count, ...) {
  va_list valist;
  va_start(valist, arg_count);
  return production_multi_(PRODUCTION_AND, arg_count, valist);
}

Production *token(const char token[]) {
  Production *p = production_create_(PRODUCTION_TOKEN);
  p->token = token;
  return p;
}

Production *optional(Production *p_child) {
  Production *p = production_create_(PRODUCTION_OPTIONAL);
  ProductionArray_init(&p->children);
  ProductionArray_push_back(&p->children, p_child);
  return p;
}

Production *newline() { return token("TOKEN_NEWLINE"); }

Production *line(Production *p) {
  Production *p_parent = production_multi_(PRODUCTION_AND, 0, NULL);
  ProductionArray_push_back(&p_parent->children, p);
  Production *nl = newline();
  ProductionArray_push_back(&p_parent->children, nl);
  return p_parent;
}

Production *epsilon() { return production_create_(PRODUCTION_EPSILON); }

Production *rule(const char rule_name[]) {
  Production *p = production_create_(PRODUCTION_RULE);
  p->rule_name = global_intern(rule_name);
  return p;
}

void production_print_(const Production *p, FILE *out) {
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
    default:  // pass
      break;
  }
  // Must be AND or OR.
  ProductionArrayIterator iter;
  ProductionArray_iterator(&iter, &p->children);
  const char *production_type = PRODUCTION_AND == p->type  ? "AND"
                                : PRODUCTION_OR == p->type ? "OR"
                                                           : "OPTIONAL";
  fprintf(out, "%s(", production_type);
  production_print_(*ProductionArray_value(&iter), out);
  ProductionArray_next(&iter);
  for (; ProductionArray_has_next(&iter); ProductionArray_next(&iter)) {
    fprintf(out, ", ");
    production_print_(*ProductionArray_value(&iter), out);
  }
  fprintf(out, ")");
}

ParserBuilder *parser_builder_create() {
  ParserBuilder *pb = malloc(sizeof(ParserBuilder));
  ProductionMap_init(&pb->rules, string_ptr_hasher_, string_ptr_comparator_);
  return pb;
}

void parser_builder_rule(ParserBuilder *pb, const char rule_name[],
                         Production *p) {
  const char *interned_rule_name = global_intern(rule_name);
  if (NULL != ProductionMap_find(&pb->rules, interned_rule_name, sizeof(char *),
                                 NULL)) {
    fprintf(stderr, "Multiple rules with name '%s'.", rule_name);
    exit(1);
  }
  ProductionMap_insert(&pb->rules, interned_rule_name, sizeof(char *), p);
}

void parser_builder_delete(ParserBuilder *pb) {
  ProductionMapIterator iter;
  ProductionMap_iterator(&iter, &pb->rules);
  for (; ProductionMap_has_entry(&iter); ProductionMap_next_entry(&iter)) {
    production_delete_(*ProductionMap_mutable_value(&iter));
  }
  ProductionMap_finalize(&pb->rules);
  free(pb);
}

const char *create_rule_function_name_(const char *production_name) {
  char buffer[128];
  int len = sprintf(buffer, "rule_%s", production_name);
  return global_intern_range(buffer, 0, len);
}

void write_rule_signature_(const char *production_name, const Production *p,
                           bool is_named_rule, FILE *file) {
  // if (!is_named_rule) {
  //   fprintf(file, "inline ");
  // }
  fprintf(file, "SyntaxTree *");
  fprintf(file, "%s", create_rule_function_name_(production_name));
  fprintf(file, "(Parser *parser)");
}

const char *suffix_for_(const Production *p) {
  return PRODUCTION_TOKEN == p->type      ? "token"
         : PRODUCTION_AND == p->type      ? "and"
         : PRODUCTION_OR == p->type       ? "or"
         : PRODUCTION_OPTIONAL == p->type ? "opt"
                                          : NULL;
}

void print_child_function_call_(const char *production_name,
                                const Production *p, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type ||
      PRODUCTION_TOKEN == p->type || PRODUCTION_OPTIONAL == p->type) {
    fprintf(file, "%s(parser);\n",
            (char *)create_rule_function_name_(production_name));
  } else if (PRODUCTION_RULE == p->type) {
    fprintf(file, "rule_%s(parser);\n", p->rule_name);
  } else if (PRODUCTION_EPSILON == p->type) {
    fprintf(file, "&MATCH_EPSILON;\n");
  } else {
    fprintf(stderr, "Unexpected production type: %d.", p->type);
    exit(1);
  }
}

const char *production_name_with_child_suffix_(const char *production_name,
                                               const Production *p,
                                               int child_index) {
  char buffer[128];
  int len =
      sprintf(buffer, "%s__%s%d", production_name, suffix_for_(p), child_index);
  return global_intern_range(buffer, 0, len);
}

bool is_helper_rule_(const char production_name[]) {
  return NULL !=
         find_str((char *)production_name, strlen(production_name), "__", 2);
}

void write_and_body_(const char *production_name, const Production *p,
                     FILE *file) {
  if (is_helper_rule_(production_name)) {
    fprintf(file, "  SyntaxTree *st = parser_create_st(parser, NULL, \"\");\n");
  } else {
    fprintf(file,
            "  SyntaxTree *st = parser_create_st(parser, rule_%s, \"%s\");\n",
            production_name, production_name);
  }
  int child_index = -1;
  ProductionArrayIterator children;
  ProductionArray_iterator(&children, &p->children);
  for (; ProductionArray_has_next(&children); ProductionArray_next(&children)) {
    ++child_index;
    const Production *p_child = *ProductionArray_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");

    if (PRODUCTION_OPTIONAL == p_child->type) {
      print_child_function_call_(production_name_with_child_suffix_(
                                     production_name, p_child, child_index),
                                 p_child, file);
      fprintf(file,
              "    if (st_child->matched) {\n"
              "       syntax_tree_add_child(st, st_child);"
              "    }\n  }\n");
    } else {
      print_child_function_call_(production_name_with_child_suffix_(
                                     production_name, p_child, child_index),
                                 p_child, file);
      fprintf(file,
              "    if (!st_child->matched) {\n"
              "      parser_delete_st(parser, st);\n"
              "      return &NO_MATCH;"
              "    }\n"
              "    syntax_tree_add_child(st, st_child);\n  }\n");
    }
  }
  fprintf(file, "  if (!st->has_children) {\n");
  fprintf(file, "    parser_delete_st(parser, st);\n");
  fprintf(file, "    return &NO_MATCH;\n  }\n");
  fprintf(file,
          "  st->matched = true;\n  return parser_prune_st(parser, st);\n");
}

void write_or_body_(const char *production_name, const Production *p,
                    FILE *file) {
  int child_index = -1;
  ProductionArrayIterator children;
  ProductionArray_iterator(&children, &p->children);
  for (; ProductionArray_has_next(&children); ProductionArray_next(&children)) {
    ++child_index;
    const Production *p_child = *ProductionArray_value(&children);
    fprintf(file, "  {\n    SyntaxTree *st_child = ");
    print_child_function_call_(production_name_with_child_suffix_(
                                   production_name, p_child, child_index),
                               p_child, file);
    fprintf(file,
            "    if (st_child->matched) {\n"
            "      if (NULL == st_child->rule_fn) {\n");
    fprintf(file, "        st_child->rule_fn = rule_%s;\n", production_name);
    fprintf(file, "        st_child->production_name = \"%s\";\n",
            production_name);
    fprintf(file,
            "      }\n"
            "      return st_child;\n    }\n  }\n");
  }
  fprintf(file, "  return &NO_MATCH;\n");
}

void write_rule_and_subrules_(const char *production_name, const Production *p,
                              bool is_named_rule, FILE *file) {
  if (PRODUCTION_AND == p->type || PRODUCTION_OR == p->type) {
    int child_index = -1;
    ProductionArrayIterator children;
    ProductionArray_iterator(&children, &p->children);
    for (; ProductionArray_has_next(&children);
         ProductionArray_next(&children)) {
      ++child_index;
      const Production *p_child = *ProductionArray_value(&children);
      if (PRODUCTION_EPSILON == p_child->type ||
          PRODUCTION_RULE == p_child->type) {
        continue;
      }
      write_rule_and_subrules_(production_name_with_child_suffix_(
                                   production_name, p_child, child_index),
                               p_child, false, file);
    }
  }
  if (PRODUCTION_OPTIONAL == p->type) {
    p = ProductionArray_get_unchecked(&p->children, 0);
    write_rule_and_subrules_(production_name, p, is_named_rule, file);
    return;
  }
  write_rule_signature_(production_name, p, is_named_rule, file);

  fprintf(file, " {\n");
  if (PRODUCTION_AND == p->type) {
    write_and_body_(production_name, p, file);
  } else if (PRODUCTION_OR == p->type) {
    write_or_body_(production_name, p, file);
  } else if (PRODUCTION_TOKEN == p->type) {
    fprintf(file,
            "  Token *token = parser_next(parser);\n"
            "  if (NULL == token || %s != token->type) {\n"
            "    return &NO_MATCH;\n  }\n",
            p->token);
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
    fprintf(stderr, "Unexpected production type: %d.", p->type);
    exit(1);
  }
  fprintf(file, "}\n\n");
}

void write_includes_(ParserBuilder *pb, FILE *file, const char h_file_path[],
                     const char lexer_h_file_path[]) {
  // Includes.
  fprintf(file,
          "#include \"%s\"\n\n"
          "#include \"language-tools/lexer/token.h\"\n"
          "#include \"%s\"\n\n",
          h_file_path, lexer_h_file_path);
}

void parser_builder_write_declare_functions_(ParserBuilder *pb, FILE *file) {
  // Internal functions not included in header must be declared at the top of
  // source.
  ProductionMapIterator rules;
  ProductionMap_iterator(&rules, &pb->rules);
  for (; ProductionMap_has_entry(&rules); ProductionMap_next_entry(&rules)) {
    const char *production_name = ProductionMap_key(&rules);
    const Production *p = *ProductionMap_value(&rules);

    fprintf(file, "// %s -> ", production_name);
    production_print_(p, file);
    fprintf(file, ";\n");

    write_rule_signature_(production_name, p, true, file);
    fprintf(file, ";\n\n");
  }
}

void parser_builder_write_c_file(ParserBuilder *pb, const char h_file_path[],
                                 const char lexer_h_file_path[], FILE *file) {
  write_includes_(pb, file, h_file_path, lexer_h_file_path);

  ProductionMapIterator rules;
  ProductionMap_iterator(&rules, &pb->rules);
  for (; ProductionMap_has_entry(&rules); ProductionMap_next_entry(&rules)) {
    const char *production_name = ProductionMap_key(&rules);
    const Production *p = *ProductionMap_value(&rules);
    write_rule_and_subrules_(production_name, p, true, file);
  }
}

void parser_builder_write_h_file(ParserBuilder *pb, FILE *file) {
  fprintf(
      file,
      "#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_TODO_THIS_H_\n"
      "#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_TODO_THIS_H_\n\n"
      "#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n"
      "#include \"language-tools/parser/parser.h\"\n\n");

  parser_builder_write_declare_functions_(pb, file);

  fprintf(file,
          "\n#ifdef __cplusplus\n}\n#endif\n\n#endif /* "
          "COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_TODO_THIS_H_ */\n");
}

void parser_builder_print(ParserBuilder *pb, FILE *out) {
  ProductionMapIterator iter;
  ProductionMap_iterator(&iter, &pb->rules);
  for (; ProductionMap_has_entry(&iter); ProductionMap_next_entry(&iter)) {
    fprintf(out, "%s -> ", ProductionMap_key(&iter));
    production_print_(*ProductionMap_value(&iter), out);
    fprintf(out, ";\n");
  }
}
