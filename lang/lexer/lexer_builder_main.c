#include <stdbool.h>
#include <stdio.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/lexer_builder.h"
#include "util/file/file_info.h"
#include "util/file/file_util.h"

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();

  bool header = 0 == strcmp("header", argv[1]);
  FILE *out_file = FILE_FN(argv[2], "w");
  FileInfo *symbols_file = file_info_file(FILE_FN(argv[3], "r"));
  FileInfo *keywords_file = file_info_file(FILE_FN(argv[4], "r"));
  FileInfo *comments_file = file_info_file(FILE_FN(argv[5], "r"));
  FileInfo *strings_file = file_info_file(FILE_FN(argv[6], "r"));

  LexerBuilder *lb = lexer_builder_create(symbols_file, keywords_file,
                                          comments_file, strings_file);

  if (header) {
    lexer_builder_write_h_file(lb, out_file);
  } else {
    char *dir_path, *file_name, *ext;
    split_path_file(argv[2], &dir_path, &file_name, &ext);
    char *h_file_path = combine_path_file(dir_path, file_name, ".h");
    lexer_builder_write_c_file(lb, out_file, intern(argv[1]));
    DEALLOC(dir_path);
    DEALLOC(file_name);
    DEALLOC(ext);
    DEALLOC(h_file_path);
  }

  fclose(out_file);

  lexer_builder_delete(lb);

  file_info_delete(symbols_file);
  file_info_delete(keywords_file);
  file_info_delete(comments_file);

  intern_finalize();
  alloc_finalize();
}