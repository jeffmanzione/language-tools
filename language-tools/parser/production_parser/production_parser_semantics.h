#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_PARSER_PRODUCTION_PARSER_PRODUCTION_PARSER_SEMANTICS_H_

#include "language-tools/lexer/token.h"
#include "language-tools/parser/production_lexer/production_lexer.h"
#include "language-tools/parser/production_parser/production_rules.h"
#include "language-tools/semantic_analyzer/expression_tree.h"
#include "language-tools/semantic_analyzer/semantic_analyzer.h"

DEFINE_EXPRESSION(epsilon) { char nan; };

DEFINE_EXPRESSION(token) { const char *token_type; };

DEFINE_EXPRESSION(rule) { const char *rule_name; };

DEFINE_EXPRESSION(and) { ExpressionTreeArray expressions; };

DEFINE_EXPRESSION(or) { ExpressionTreeArray expressions; };

DEFINE_EXPRESSION(optional) { ExpressionTree *expression; };

DEFINE_EXPRESSION(sequence) {
  ExpressionTree *delim;
  ExpressionTree *item;
};

DEFINE_EXPRESSION(production_rule) {
  const char *rule_name;
  ExpressionTree *expression;
};

DEFINE_EXPRESSION(production_rule_set) { ExpressionTreeArray rules; };

void production_parser_init_semantics(SAMap *populators, SAMap *producers,
                                      SAMap *deleters);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_..._PRODUCTION_PARSER_SEMANTICs_H_*/