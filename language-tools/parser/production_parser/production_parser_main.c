#include "file-utils/file_info.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/token.h"
#include "language-tools/parser/parser_builder.h"
#include "language-tools/parser/production_lexer/production_lexer.h"
#include "language-tools/parser/production_parser/production_parser_semantics.h"
#include "language-tools/parser/production_parser/production_rules.h"
#include "language-tools/semantic_analyzer/expression_tree.h"
#include "language-tools/semantic_analyzer/semantic_analyzer.h"

Production *produce_production_(ParserBuilder *pb, const char rule_name[],
                                const ExpressionTree *etree) {
  if (IS_EXPRESSION(etree, epsilon)) {
    return epsilon();
  }
  if (IS_EXPRESSION(etree, token)) {
    Expression_token *t = EXTRACT_EXPRESSION(etree, token);
    return token(t->token_type);
  }
  if (IS_EXPRESSION(etree, rule)) {
    Expression_rule *r = EXTRACT_EXPRESSION(etree, rule);
    return rule(r->rule_name);
  }
  if (IS_EXPRESSION(etree, optional)) {
    Expression_optional *o = EXTRACT_EXPRESSION(etree, optional);
    return optional(produce_production_(pb, rule_name, o->expression));
  }
  if (IS_EXPRESSION(etree, and)) {
    Expression_and *a = EXTRACT_EXPRESSION(etree, and);
    Production *p = production_and();
    ExpressionTreeArrayIterator expressions;
    ExpressionTreeArray_iterator(&expressions, &a->expressions);
    for (; ExpressionTreeArray_has_next(&expressions);
         ExpressionTreeArray_next(&expressions)) {
      production_add_child(
          p, produce_production_(pb, rule_name,
                                 *ExpressionTreeArray_value(&expressions)));
    }
    return p;
  }
  if (IS_EXPRESSION(etree, or)) {
    Expression_or *o = EXTRACT_EXPRESSION(etree, or);
    Production *p = production_or();
    ExpressionTreeArrayIterator expressions;
    ExpressionTreeArray_iterator(&expressions, &o->expressions);
    for (; ExpressionTreeArray_has_next(&expressions);
         ExpressionTreeArray_next(&expressions)) {
      production_add_child(
          p,
          produce_production_(
              pb, rule_name, *ExpressionTreeArray_mutable_value(&expressions)));
    }
    return p;
  }
  if (IS_EXPRESSION(etree, sequence)) {
    Expression_sequence *s = EXTRACT_EXPRESSION(etree, sequence);

    char buffer[128];
    int len = sprintf(buffer, "%s1", rule_name);
    const char *helper_rule_name = global_intern_range(buffer, 0, len);

    Production *p_helper_and = production_and();
    if (NULL != s->delim) {
      production_add_child(p_helper_and,
                           produce_production_(pb, helper_rule_name, s->delim));
    }
    production_add_child(p_helper_and,
                         produce_production_(pb, helper_rule_name, s->item));
    production_add_child(p_helper_and, rule(helper_rule_name));
    Production *p_helper = production_or();
    production_add_child(p_helper, p_helper_and);
    production_add_child(p_helper, epsilon());

    parser_builder_rule(pb, helper_rule_name, p_helper);

    Production *p = production_and();
    production_add_child(p, produce_production_(pb, helper_rule_name, s->item));
    production_add_child(p, rule(helper_rule_name));
    return p;
  }
  fprintf(stderr, "Unknown Expression type.");
  exit(1);
  return NULL;
}

void produce_parser_builder_(ParserBuilder *pb, const ExpressionTree *etree) {
  ExpressionTreeArrayIterator rules;
  ExpressionTreeArray_iterator(
      &rules, &((Expression_production_rule_set *)etree->expression)->rules);
  for (; ExpressionTreeArray_has_next(&rules);
       ExpressionTreeArray_next(&rules)) {
    const ExpressionTree *exp = *ExpressionTreeArray_value(&rules);
    const Expression_production_rule *rule =
        EXTRACT_EXPRESSION(exp, production_rule);
    parser_builder_rule(
        pb, rule->rule_name,
        produce_production_(pb, rule->rule_name, rule->expression));
  }
}

int main(int argc, const char *argv[]) {
  global_string_intern_pool_init();

  TokenArray tokens;
  TokenArray_init(&tokens);

  FileInfo *fi = file_info(argv[1]);
  const bool header = 0 == strcmp("header", argv[2]);
  FILE *out_file = fopen(argv[3], "w");

  lexer_tokenize(fi, &tokens);

  Parser parser;
  parser_init(&parser, rule_production_rule_set, /*ignore_newline=*/true);

  SyntaxTree *productions = parser_parse(&parser, &tokens);

  if (TokenArray_size(&tokens) > 0) {
    TokenArrayIterator iter;
    TokenArray_iterator(&iter, &tokens);
    for (; TokenArray_has_next(&iter); TokenArray_next(&iter)) {
      printf("  '%s'\n", (*TokenArray_value(&iter))->text);
    }
    fprintf(stderr, "EXTRA TOKENS NOT PARSED.\n");
    exit(1);
  }

  SemanticAnalyzer analyzer;
  semantic_analyzer_init(&analyzer, production_parser_init_semantics);

  ExpressionTree *etree = semantic_analyzer_populate(&analyzer, productions);

  ParserBuilder *pb =
      parser_builder_create((TokenToStringFn)token_type_to_name,
                            (StringToTokenFn)token_name_to_token_type);

  produce_parser_builder_(pb, etree);

  if (header) {
    parser_builder_write_h_file(pb, out_file);
  } else {
    const char *h_file = global_intern(argv[4]);
    const char *lexer_h_file = global_intern(argv[5]);
    parser_builder_write_c_file(pb, h_file, lexer_h_file, out_file);
  }

  fclose(out_file);

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  // parser_builder_delete(pb);
  // semantic_analyzer_delete(&analyzer, etree);
  // semantic_analyzer_finalize(&analyzer);
  // parser_delete_st(&parser, productions);
  // parser_finalize(&parser);
  // TokenArray_finalize(&tokens);
  // file_info_delete(fi);
  // token_finalize_all();
  // global_string_intern_pool_finalize();

  return 0;
}