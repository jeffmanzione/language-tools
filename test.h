#ifndef LANG_PARSER_TODO_THIS_H_
#define LANG_PARSER_TODO_THIS_H_

#include "lang/parser/parser.h"

// epsilon -> token:KEYWORD_EPSILON;
SyntaxTree *rule_epsilon(Parser *parser);
// token -> and(token:KEYWORD_TOKEN, token:SYMBOL_COLON, token:TOKEN_WORD);
SyntaxTree *rule_token(Parser *parser);
// rule -> and(token:KEYWORD_RULE, token:SYMBOL_COLON, token:TOKEN_WORD);
SyntaxTree *rule_rule(Parser *parser);
// list1 -> or(and(token:SYMBOL_COMMA, rule:production_expression, rule:list1), E);
SyntaxTree *rule_list1(Parser *parser);
// list -> and(rule:production_expression, rule:list1);
SyntaxTree *rule_list(Parser *parser);
// and -> and(token:KEYWORD_AND, token:SYMBOL_LPAREN, rule:list, token:SYMBOL_RPAREN);
SyntaxTree *rule_and(Parser *parser);
// or -> and(token:KEYWORD_OR, token:SYMBOL_LPAREN, rule:list, token:SYMBOL_RPAREN);
SyntaxTree *rule_or(Parser *parser);
// production_expression -> or(rule:and, rule:or, rule:rule, rule:token, rule:epsilon);
SyntaxTree *rule_production_expression(Parser *parser);
// production_rule -> and(token:TOKEN_WORD, token:SYMBOL_ARROW, rule:production_expression, token:SYMOBL_SEMICOLON);
SyntaxTree *rule_production_rule(Parser *parser);

#endif /* LANG_PARSER_TODO_THIS_H_ */
