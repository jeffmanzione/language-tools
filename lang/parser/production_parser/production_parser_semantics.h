#ifndef LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_
#define LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_

#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_rules.h"
#include "lang/semantic_analyzer/expression_tree.h"
#include "lang/semantic_analyzer/semantic_analyzer.h"
#include "struct/q.h"

DEFINE_EXPRESSION(epsilon) { char nan; };

DEFINE_EXPRESSION(token) { const char *token_type; };

DEFINE_EXPRESSION(rule) { const char *rule_name; };

DEFINE_EXPRESSION(and) { AList expressions; };

DEFINE_EXPRESSION(or) { AList expressions; };

DEFINE_EXPRESSION(optional) { ExpressionTree *expression; };

DEFINE_EXPRESSION(production_rule) {
  const char *rule_name;
  ExpressionTree *expression;
};

DEFINE_EXPRESSION(production_rule_set) { AList rules; };

void production_parser_init_semantics(Map *populators, Map *deleters);

#endif /* LANGUAGE_TOOLS_LANG_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_ \
        */