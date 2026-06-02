#include "language-tools/intern.h"
#include "language-tools/parser/parser_builder.h"
#include "language-tools/parser/production_lexer/production_lexer.h"

ParserBuilder *create_parser_builder_() {
  ParserBuilder *pb = parser_builder_create();

  parser_builder_rule(pb, "epsilon", token("KEYWORD_EPSILON"));
  parser_builder_rule(
      pb, "token",
      and3(token("KEYWORD_TOKEN"), token("SYMBOL_COLON"), token("TOKEN_WORD")));
  parser_builder_rule(
      pb, "rule",
      and3(token("KEYWORD_RULE"), token("SYMBOL_COLON"), token("TOKEN_WORD")));
  parser_builder_rule(pb, "list1",
                      or2(and3(token("SYMBOL_COMMA"),
                               rule("production_expression"), rule("list1")),
                          epsilon()));
  parser_builder_rule(pb, "list",
                      and2(rule("production_expression"), rule("list1")));
  parser_builder_rule(pb, "and",
                      and4(token("KEYWORD_AND"), token("SYMBOL_LPAREN"),
                           rule("list"), token("SYMBOL_RPAREN")));
  parser_builder_rule(pb, "or",
                      and4(token("KEYWORD_OR"), token("SYMBOL_LPAREN"),
                           rule("list"), token("SYMBOL_RPAREN")));
  parser_builder_rule(
      pb, "optional",
      and4(token("KEYWORD_OPTIONAL"), token("SYMBOL_LPAREN"),
           rule("production_expression"), token("SYMBOL_RPAREN")));
  parser_builder_rule(
      pb, "sequence",
      or2(and6(token("KEYWORD_LIST"), token("SYMBOL_LPAREN"),
               rule("production_expression"), token("SYMBOL_COMMA"),
               rule("production_expression"), token("SYMBOL_RPAREN")),
          and4(token("KEYWORD_LIST"), token("SYMBOL_LPAREN"),
               rule("production_expression"), token("SYMBOL_RPAREN"))));
  parser_builder_rule(
      pb, "production_expression",
      or7(rule("optional"), rule("and"), rule("or"), rule("sequence"),
          rule("rule"), rule("token"), rule("epsilon")));
  parser_builder_rule(
      pb, "production_rule",
      and4(token("TOKEN_WORD"), token("SYMBOL_ARROW"),
           rule("production_expression"), token("SYMOBL_SEMICOLON")));
  parser_builder_rule(
      pb, "production_rule_set1",
      or2(and2(rule("production_rule"), rule("production_rule_set1")),
          epsilon()));
  parser_builder_rule(
      pb, "production_rule_set",
      and2(rule("production_rule"), rule("production_rule_set1")));
  return pb;
}

int main(int argc, const char *argv[]) {
  global_string_intern_pool_init();

  bool header = 0 == strcmp("header", argv[1]);

  ParserBuilder *pb = create_parser_builder_();

  if (header) {
    parser_builder_write_h_file(pb, stdout);
  } else {
    parser_builder_write_c_file(pb, argv[2], argv[3], stdout);
  }

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  //   parser_builder_delete(pb);
  //   global_string_intern_pool_finalize();

  return 0;
}