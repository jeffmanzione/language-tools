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
  parser_builder_rule(pb, "production_expression",
                      or5(rule("and"), rule("or"), rule("rule"), rule("token"),
                          rule("epsilon")));
  parser_builder_rule(
      pb, "production_rule",
      and4(token(TOKEN_WORD), token(SYMBOL_ARROW),
           rule("production_expression"), token(SYMOBL_SEMICOLON)));
  return pb;
}

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  bool header = 0 == strcmp("header", argv[1]);
  FILE *out_file = FILE_FN(argv[2], "w");

  ParserBuilder *pb = _create_parser_builder();

  if (header) {
    parser_builder_write_h_file(pb, (TokenToStringFn)token_type_to_name,
                                out_file);
  } else {
    char *dir_path, *file_name, *ext;
    split_path_file(argv[2], &dir_path, &file_name, &ext);
    char *h_file_path = combine_path_file(dir_path, file_name, ".h");
    parser_builder_write_c_file(pb, (TokenToStringFn)token_type_to_name,
                                h_file_path, argv[3], out_file);
  }
  fclose(out_file);

  parser_builder_delete(pb);

  intern_finalize();
  alloc_finalize();
  return 0;
}