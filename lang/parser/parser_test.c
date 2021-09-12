#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/test_lexer.h"
#include "lang/parser/parser_builder.h"

int main(int argc, const char *args[]) {
  alloc_init();
  intern_init();

  ParserBuilder *pb = parser_builder_create();

  parser_builder_rule(pb, "identifier", token(TOKEN_WORD));

  parser_builder_rule(pb, "or1",
                      or2(and2(token(KEYWORD_OR), rule("or")), epsilon()));
  parser_builder_rule(pb, "or", and2(rule("identifier"), rule("or1")));

  parser_builder_set_root(pb, rule("or"));

  printf("%s", token_type_to_str(1));

  parser_builder_print(pb, token_type_to_str, stdout);

  parser_builder_delete(pb);

  intern_finalize();
  alloc_finalize();
  return 0;
}