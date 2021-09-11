#include <stdbool.h>

#include "alloc/arena/intern.h"
#include "lang/lexer/test_lexer.h"
#include "lang/lexer/token.h"
#include "struct/q.h"
#include "util/file/file_info.h"
#include "util/file/sfile.h"

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();
  const char test[] = "Dog dog = Dog(/* name */ \'Spike', /* weight */ 25.4, "
                      "4, 5f);\n\ndog.bark();";
  Q tokens;
  Q_init(&tokens);

  SFILE *sfile = sfile_open(test);
  FileInfo *fi = file_info_sfile(sfile);
  lexer_tokenize(fi, &tokens);

  Q_iter iter = Q_iterator(&tokens);
  for (; Q_has(&iter); Q_inc(&iter)) {
    Token *token = *((Token **)Q_value(&iter));
    printf("token %d '%s'\n", token->type, token->text);
  }
  Q_finalize(&tokens);

  sfile_close(sfile);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();
  return 0;
}