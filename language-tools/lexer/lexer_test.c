#include <stdbool.h>

#include "file-utils/file_info.h"
#include "file-utils/sfile.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/test_lexer.h"
#include "language-tools/lexer/token.h"

int main(int argc, const char *argv[]) {
  global_string_intern_pool_init();

  const char test[] =
      "Dog dog = Dog(/* name */ \'Spike', /* weight */ 25.4, "
      "4, 5f);\n\ndog.bark();";
  TokenArray tokens;
  TokenArray_init(&tokens);

  SFILE *sfile = sfile_open(test);
  FileInfo *fi = file_info_sfile(sfile);
  lexer_tokenize(fi, &tokens);

  TokenArrayIterator iter;
  TokenArray_iterator(&iter, &tokens);
  for (; TokenArray_has_next(&iter); TokenArray_next(&iter)) {
    const Token *token = *((Token **)TokenArray_value(&iter));
    printf("token %d '%s'\n", token->type, token->text);
  }

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  // TokenArray_finalize(&tokens);
  // sfile_close(sfile);
  // token_finalize_all();
  // global_string_intern_pool_finalize();

  return 0;
}