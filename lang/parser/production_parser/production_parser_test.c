#include "alloc/arena/intern.h"
#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_rules.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "lang/semantic_analyzer/semantic_analyzer.h"
#include "struct/q.h"
#include "util/file/file_info.h"

DefineExpression(production_rule) { int a; };

ImplPopulate(production_rule, const SyntaxTree *stree) {
  production_rule->a = 1;
}

ImplDelete(production_rule) {}

DefineExpression(production_rule_set) { int b; };

ImplPopulate(production_rule_set, const SyntaxTree *stree) {
  production_rule_set->b = 352;
}

ImplDelete(production_rule_set) {}

void _init_semantics(Map *populators, Map *deleters) {
  Register(production_rule);
  Register(production_rule_set);
}

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  Q tokens;
  Q_init(&tokens);

  FileInfo *fi = file_info(argv[1]);
  lexer_tokenize(fi, &tokens);

  // Q_iter iter = Q_iterator(&tokens);
  // for (; Q_has(&iter); Q_inc(&iter)) {
  //   Token *token = *((Token **)Q_value(&iter));
  //   printf("token %s '%s'\n", token_type_to_name(token->type), token->text);
  //   fflush(stdout);
  // }

  Parser parser;
  parser_init(&parser, rule_production_rule_set);

  SyntaxTree *productions = parser_parse(&parser, &tokens);
  syntax_tree_print(productions, 0, stdout);

  SemanticAnalyzer analyzer;
  semantic_analyzer_init(&analyzer, _init_semantics);

  ExpressionTree *etree = semantic_analyzer_populate(&analyzer, productions);
  printf("%d\n", ((Expression_production_rule_set *)etree->expression)->b);
  semantic_analyzer_delete(&analyzer, etree);

  semantic_analyzer_finalize(&analyzer);

  file_info_delete(fi);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();

  return 0;
}