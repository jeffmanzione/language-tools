#include "alloc/arena/intern.h"
#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"
#include "lang/parser/production_parser/production_rules.h"
#include "struct/q.h"
#include "util/file/file_info.h"

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  Q tokens;
  Q_init(&tokens);

  FileInfo *fi = file_info(argv[1]);
  lexer_tokenize(fi, &tokens);

  // Q_iter iter = Q_iterator(&tokens);
  // for (; Q_has(&iter); Q_inc(&iter)) {
  //   Token *token = *((Token **)Q_value(&iter));
  //   printf("token %s '%s'\n", token_type_to_name(token->type), token->text);
  //   fflush(stdout);
  // }

  Parser parser;
  parser_init(&parser, rule_production_rule_set);

  SyntaxTree *productions = parser_parse(&parser, &tokens);
  syntax_tree_print(productions, 0, stdout);

  file_info_delete(fi);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();

  return 0;
}