#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/test_lexer.h"
#include "lang/parser/parser_builder.h"

int main(int argc, const char *args[]) {
  alloc_init();
  intern_init();

  ParserBuilder *pb = parser_builder_create();

  parser_builder_rule(pb, "identifier", token(TOKEN_WORD));
  parser_builder_rule(pb, "constant",
                      or2(token(TOKEN_INTEGER), token(TOKEN_FLOATING)));
  parser_builder_rule(pb, "string_literal", token(TOKEN_STRING));
  parser_builder_rule(pb, "array_declaration",
                      or2(and2(token(SYMBOL_LBRACKET), token(SYMBOL_RBRACKET)),
                          and3(token(SYMBOL_RBRACKET), rule("tuple_expression"),
                               token(SYMBOL_RBRACKET))));
  parser_builder_rule(
      pb, "primary_expression",
      or4(rule("identifier"), rule("constant"), rule("string_literal"),
          and3(token(SYMBOL_LPAREN), rule("tuple_expression"),
               token(SYMBOL_RPAREN))));
  parser_builder_rule(pb, "and1",
                      or2(and2(token(KEYWORD_AND), rule("and")), epsilon()));
  parser_builder_rule(pb, "and",
                      and2(rule("primary_expression"), rule("and1")));
  parser_builder_rule(pb, "or1",
                      or2(and2(token(KEYWORD_OR), rule("or")), epsilon()));
  parser_builder_rule(pb, "or", and2(rule("and"), rule("or1")));
  parser_builder_rule(
      pb, "tuple_expression1",
      or2(and3(token(SYMBOL_COMMA), rule("or"), rule("tuple_expression1")),
          epsilon()));
  parser_builder_rule(pb, "tuple_expression",
                      and2(rule("or"), rule("tuple_expression1")));

  parser_builder_set_root(pb, rule("tuple_expression"));

  parser_builder_write_h_file(pb, (TokenToStringFn)token_type_to_name, stdout);
  parser_builder_write_c_file(pb, (TokenToStringFn)token_type_to_name, stdout);

  parser_builder_delete(pb);

  intern_finalize();
  alloc_finalize();
  return 0;
}