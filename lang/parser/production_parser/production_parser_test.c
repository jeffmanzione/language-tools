#include "alloc/arena/intern.h"
#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_rules.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "lang/semantic_analyzer/semantic_analyzer.h"
#include "struct/q.h"
#include "util/file/file_info.h"

DefineExpression(production_rule) {
  const char *rule_name;
  ExpressionTree *production;
};

DefineExpression(production_rule_set) { AList rules; };

ImplPopulate(production_rule, const SyntaxTree *stree,
             SemanticAnalyzer *analyzer) {
  production_rule->production = NULL;
}

ImplDelete(production_rule, SemanticAnalyzer *analyzer) {
  if (NULL != production_rule->production) {
    semantic_analyzer_delete(analyzer, production_rule->production);
  }
}

ImplPopulate(production_rule_set, const SyntaxTree *stree,
             SemanticAnalyzer *analyzer) {
  alist_init(&production_rule_set->rules, ExpressionTree *, DEFAULT_ARRAY_SZ);
  const SyntaxTree *first = *(SyntaxTree **)alist_get(&stree->children, 0);
  ExpressionTree *first_rule = semantic_analyzer_populate(analyzer, first);
  alist_append(&production_rule_set->rules, &first_rule);
  
}

ImplDelete(production_rule_set, SemanticAnalyzer *analyzer) {
  AL_iter iter = alist_iter(&production_rule_set->rules);
  for (; al_has(&iter); al_inc(&iter)) {
    semantic_analyzer_delete(analyzer, *(ExpressionTree **)al_value(&iter));
  }
  alist_finalize(&production_rule_set->rules);
}

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
  semantic_analyzer_delete(&analyzer, etree);

  semantic_analyzer_finalize(&analyzer);

  file_info_delete(fi);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();
  return 0;
}