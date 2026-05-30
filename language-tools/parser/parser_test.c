#include "language-tools/parser/parser.h"

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "language-tools/lexer/test_lexer.h"
#include "language-tools/lexer/token.h"
#include "language-tools/parser/test_parser.h"
#include "struct/q.h"
#include "util/file/file_info.h"
#include "util/file/sfile.h"

#define BUFFER_SIZE 256

int main(int argc, const char *args[]) {
  alloc_init();
  intern_init();

  FileInfo *fi = file_info_file(stdin);

  while (true) {
    Q tokens;
    Q_init(&tokens);

    printf("> ");

    lexer_tokenize_line(fi, &tokens);

    Parser parser;
    parser_init(&parser, rule_tuple_expression, /*ignore_newline=*/false);

    const SyntaxTree *stree = parser_parse(&parser, &tokens);
    syntax_tree_print(stree, 0, stdout);
    printf("\n");

    Q_iter iter = Q_iterator(&tokens);
    for (; Q_has(&iter); Q_inc(&iter)) {
      Token *token = *((Token **)Q_value(&iter));
      if (token->type != TOKEN_NEWLINE) {
        printf("EXTRA TOKEN %d '%s'\n", token->type, token->text);
      }
    }

    parser_finalize(&parser);

    Q_finalize(&tokens);
  }

  intern_finalize();
  alloc_finalize();
  return 0;
}