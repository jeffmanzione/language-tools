#include "lang/lexer/test_lexer.h"
#include "lang/parser/parser_builder.h"

int main(int arc, const char *args[]) {
  ParserBuilder *pb = parser_builder_create();

  Production *root =
      and(token(TOKEN_WORD), token(KEYWORD_OR), token(TOKEN_WORD));

  parser_builder_set_root(pb, root);
  parser_builder_delete(pb);
  return 0;
}