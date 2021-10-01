#include "alloc/arena/intern.h"
#include "lang/lexer/token.h"
#include "lang/parser/parser_builder.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_parser_semantics.h"
#include "lang/parser/production_parser/production_rules.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "lang/semantic_analyzer/semantic_analyzer.h"
#include "struct/q.h"
#include "util/file/file_info.h"

Production *_produce_production(const ExpressionTree *etree) {
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
  if (IS_EXPRESSION(etree, and)) {
    Expression_and *a = EXTRACT_EXPRESSION(etree, and);
    Production *p = production_and();
    AL_iter expressions = alist_iter(&a->expressions);
    for (; al_has(&expressions); al_inc(&expressions)) {
      production_add_child(
          p, _produce_production(*(ExpressionTree **)al_value(&expressions)));
    }
    return p;
  }
  if (IS_EXPRESSION(etree, or)) {
    Expression_or *o = EXTRACT_EXPRESSION(etree, or);
    Production *p = production_or();
    AL_iter expressions = alist_iter(&o->expressions);
    for (; al_has(&expressions); al_inc(&expressions)) {
      production_add_child(
          p, _produce_production(*(ExpressionTree **)al_value(&expressions)));
    }
    return p;
  }
  ERROR("Unknown Expression type.");
  return NULL;
}

void _produce_parser_builder(ParserBuilder *pb, const ExpressionTree *etree) {
  AL_iter rules =
      alist_iter(&((Expression_production_rule_set *)etree->expression)->rules);
  for (; al_has(&rules); al_inc(&rules)) {
    const ExpressionTree *exp = *(ExpressionTree **)al_value(&rules);
    const Expression_production_rule *rule =
        EXTRACT_EXPRESSION(exp, production_rule);
    parser_builder_rule(pb, rule->rule_name,
                        _produce_production(rule->expression));
  }
}

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  Q tokens;
  Q_init(&tokens);

  FileInfo *fi = file_info(argv[1]);
  const bool header = 0 == strcmp("header", argv[2]);
  lexer_tokenize(fi, &tokens);

  Parser parser;
  parser_init(&parser, rule_production_rule_set);

  SyntaxTree *productions = parser_parse(&parser, &tokens);

  SemanticAnalyzer analyzer;
  semantic_analyzer_init(&analyzer, production_parser_init_semantics);

  ExpressionTree *etree = semantic_analyzer_populate(&analyzer, productions);

  ParserBuilder *pb =
      parser_builder_create((TokenToStringFn)token_type_to_name,
                            (StringToTokenFn)token_name_to_token_type);

  _produce_parser_builder(pb, etree);

  if (header) {
    parser_builder_write_h_file(pb, stdout);
  } else {
    const char *h_file = intern(argv[3]);
    const char *lexer_h_file = intern(argv[4]);
    parser_builder_write_c_file(pb, h_file, lexer_h_file, stdout);
  }

  parser_builder_delete(pb);

  semantic_analyzer_delete(&analyzer, etree);

  semantic_analyzer_finalize(&analyzer);

  parser_delete_st(&parser, productions);
  parser_finalize(&parser);
  Q_finalize(&tokens);

  file_info_delete(fi);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();
  return 0;
}