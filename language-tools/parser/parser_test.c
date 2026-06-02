#include "language-tools/parser/parser.h"

#include "file-utils/file_info.h"
#include "file-utils/sfile.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/test_lexer.h"
#include "language-tools/lexer/token.h"
#include "language-tools/parser/test_parser.h"

#define BUFFER_SIZE 256

int main(int argc, const char *args[]) {
  global_string_intern_pool_init();

  FileInfo *fi = file_info_file(stdin);

  while (true) {
    TokenArray tokens;
    TokenArray_init(&tokens);

    printf("> ");

    lexer_tokenize_line(fi, &tokens);

    Parser parser;
    parser_init(&parser, rule_tuple_expression, /*ignore_newline=*/false);

    const SyntaxTree *stree = parser_parse(&parser, &tokens);

    syntax_tree_print(stree, 0, stdout);
    printf("\n");

    TokenArrayIterator iter;
    TokenArray_iterator(&iter, &tokens);
    for (; TokenArray_has_next(&iter); TokenArray_next(&iter)) {
      const Token *token = *TokenArray_value(&iter);
      if (token->type != TOKEN_NEWLINE) {
        printf("EXTRA TOKEN %d '%s'\n", token->type, token->text);
      }
    }

    parser_finalize(&parser);

    TokenArray_finalize(&tokens);
  }

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  // global_string_intern_pool_finalize();

  return 0;
}