#include <stdbool.h>
#include <stdio.h>

#include "file-utils/file_info.h"
#include "file-utils/file_utils.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/lexer_builder.h"

int main(int argc, const char *argv[]) {
  global_string_intern_pool_init();

  const bool header = 0 == strcmp("header", argv[1]);
  FILE *out_file = FILE_FN(argv[2], "w");
  FileInfo *symbols_file = file_info_file(FILE_FN(argv[3], "r"));
  FileInfo *keywords_file = file_info_file(FILE_FN(argv[4], "r"));
  FileInfo *comments_file = file_info_file(FILE_FN(argv[5], "r"));
  FileInfo *strings_file = file_info_file(FILE_FN(argv[6], "r"));

  LexerBuilder lb;
  lexer_builder_init(&lb, symbols_file, keywords_file, comments_file,
                     strings_file);

  if (header) {
    lexer_builder_write_h_file(&lb, out_file);
  } else {
    char *dir_path, *file_name, *ext;
    split_path_file(argv[2], &dir_path, &file_name, &ext);
    lexer_builder_write_c_file(&lb, out_file, global_intern(argv[1]));

    // Below code not necessary as the program will immediately free all memory
    // upon exit.

    // free(dir_path);
    // free(file_name);
    // free(ext);
  }

  fclose(out_file);

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  // lexer_builder_finalize(&lb);

  // file_info_delete(symbols_file);
  // file_info_delete(keywords_file);
  // file_info_delete(comments_file);

  // global_string_intern_pool_finalize();
}