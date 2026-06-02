#include <stdbool.h>
#include <stdio.h>

#include "file-utils/file_info.h"
#include "file-utils/file_utils.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/lexer_builder.h"

int main(int argc, const char *argv[]) {
  global_string_intern_pool_init();

  const char *src_header_path = global_intern(argv[1]);
  const char *out_file_path = global_intern(argv[2]);
  FileInfo *symbols_file = file_info_file(FILE_FN(argv[3], "r"));
  FileInfo *keywords_file = file_info_file(FILE_FN(argv[4], "r"));
  FileInfo *comments_file = file_info_file(FILE_FN(argv[5], "r"));
  FileInfo *strings_file = file_info_file(FILE_FN(argv[6], "r"));
  const char *fn_prefix = strlen(argv[7]) == 0 ? "" : global_intern(argv[7]);
  const char *enum_prefix = strlen(argv[8]) == 0 ? "" : global_intern(argv[8]);

  FILE *out_file = FILE_FN(out_file_path, "w");

  LexerBuilder lb;
  lexer_builder_init(&lb, symbols_file, keywords_file, comments_file,
                     strings_file);

  const bool is_header = 0 == strcmp("header", src_header_path);
  if (is_header) {
    lexer_builder_write_h_file(&lb, out_file, fn_prefix, enum_prefix);
  } else {
    char *dir_path, *file_name, *ext;
    split_path_file(out_file_path, &dir_path, &file_name, &ext);
    lexer_builder_write_c_file(&lb, out_file, src_header_path, fn_prefix,
                               enum_prefix);

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