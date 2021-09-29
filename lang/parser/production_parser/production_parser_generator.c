#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/parser/parser_builder.h"
#include "lang/parser/production_lexer/production_lexer.h"

ParserBuilder *_create_parser_builder() {
  ParserBuilder *pb = parser_builder_create();

  parser_builder_rule(pb, "epsilon", token(KEYWORD_EPSILON));
  parser_builder_rule(
      pb, "token",
      and3(token(KEYWORD_TOKEN), token(SYMBOL_COLON), token(TOKEN_WORD)));
  parser_builder_rule(
      pb, "rule",
      and3(token(KEYWORD_RULE), token(SYMBOL_COLON), token(TOKEN_WORD)));
  parser_builder_rule(pb, "list1",
                      or2(and3(token(SYMBOL_COMMA),
                               rule("production_expression"), rule("list1")),
                          epsilon()));
  parser_builder_rule(pb, "list",
                      and2(rule("production_expression"), rule("list1")));
  parser_builder_rule(pb, "and",
                      and4(token(KEYWORD_AND), token(SYMBOL_LPAREN),
                           rule("list"), token(SYMBOL_RPAREN)));
  parser_builder_rule(pb, "or",
                      and4(token(KEYWORD_OR), token(SYMBOL_LPAREN),
                           rule("list"), token(SYMBOL_RPAREN)));
  parser_builder_rule(
      pb, "optional",
      and4(token(KEYWORD_OPTIONAL), token(SYMBOL_LPAREN),
           rule("production_expression"), token(SYMBOL_RPAREN)));
  parser_builder_rule(pb, "production_expression",
                      or6(rule("optional"), rule("and"), rule("or"),
                          rule("rule"), rule("token"), rule("epsilon")));
  parser_builder_rule(pb, "production_rule",
                      and3(token(TOKEN_WORD), token(SYMBOL_ARROW),
                           rule("production_expression")));
  parser_builder_rule(
      pb, "production_rule_set1",
      or2(and4(token(SYMOBL_SEMICOLON), optional(token(TOKEN_NEWLINE)),
               rule("production_rule"), rule("production_rule_set1")),
          epsilon()));
  parser_builder_rule(
      pb, "production_rule_set",
      and2(rule("production_rule"), rule("production_rule_set1")));
  return pb;
}

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  bool header = 0 == strcmp("header", argv[1]);

  ParserBuilder *pb = _create_parser_builder();

  if (header) {
    parser_builder_write_h_file(pb, (TokenToStringFn)token_type_to_name,
                                stdout);
  } else {
    parser_builder_write_c_file(pb, (TokenToStringFn)token_type_to_name,
                                argv[2], argv[3], stdout);
  }

  parser_builder_delete(pb);

  intern_finalize();
  alloc_finalize();
  return 0;
}