#ifndef LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_
#define LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_

#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_rules.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "lang/semantic_analyzer/semantic_analyzer.h"
#include "struct/q.h"

DefineExpression(epsilon) { char nan; };

DefineExpression(token) { const char *token_type; };

DefineExpression(rule) { const char *rule_name; };

DefineExpression(and) { AList expressions; };

DefineExpression(or) { AList expressions; };

DefineExpression(optional) { ExpressionTree *expression; };

DefineExpression(production_rule) {
  const char *rule_name;
  ExpressionTree *expression;
};

DefineExpression(production_rule_set) { AList rules; };

void production_parser_init_semantics(Map *populators, Map *deleters);

#endif /* LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_ \
        */