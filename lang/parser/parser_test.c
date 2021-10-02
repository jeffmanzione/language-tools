#include "lang/parser/parser.h"

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/test_lexer.h"
#include "lang/lexer/token.h"
#include "lang/parser/test_parser.h"
#include "struct/q.h"
#include "util/file/file_info.h"
#include "util/file/sfile.h"

int main(int argc, const char *args[]) {
  alloc_init();
  intern_init();

  const char test[] = "7 and (a, (cat or dog))";
  Q tokens;
  Q_init(&tokens);

  SFILE *sfile = sfile_open(test);
  FileInfo *fi = file_info_sfile(sfile);
  lexer_tokenize(fi, &tokens);

  //   Q_iter iter = Q_iterator(&tokens);
  //   for (; Q_has(&iter); Q_inc(&iter)) {
  //     Token *token = *((Token **)Q_value(&iter));
  //     printf("token %d '%s'\n", token->type, token->text);
  //   }

  Parser parser;
  parser_init(&parser, rule_tuple_expression);

  const SyntaxTree *stree = parser_parse(&parser, &tokens);
  syntax_tree_print(stree, 0, stdout);
  printf("\n");

  parser_finalize(&parser);

  Q_finalize(&tokens);

  intern_finalize();
  alloc_finalize();
  return 0;
}