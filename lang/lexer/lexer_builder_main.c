
#include <stdio.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/lexer_builder.h"
#include "util/file/file_info.h"
#include "util/file/file_util.h"

int main(int argc, const char *argv[]) {
  alloc_init();
  intern_init();
  FILE *c_out_file = FILE_FN(argv[1], "w");
  FileInfo *symbols_file = file_info_file(FILE_FN(argv[2], "r"));
  FileInfo *keywords_file = file_info_file(FILE_FN(argv[3], "r"));
  FileInfo *comments_file = file_info_file(FILE_FN(argv[4], "r"));

  LexerBuilder *lb =
      lexer_builder_create(symbols_file, keywords_file, comments_file);

  lexer_builder_write_c_file(lb, c_out_file);
  fclose(c_out_file);

  lexer_builder_delete(lb);

  file_info_delete(symbols_file);
  file_info_delete(keywords_file);
  file_info_delete(comments_file);

  intern_finalize();
  alloc_finalize();
}

// void _lexer_tokenize_line(FileInfo *fi, Q *tokens) {
//   LineInfo *li = file_info_getline(fi);
//   if (NULL == li) {
//     return;
//   }
//   int col_num = 0, row_num = 0;
//   char *line = li->line_text;
//   while (true) {
//   }
// }

// void lexer_tokenize(FileInfo *file, Q *tokens) {
//   ASSERT(NOT_NULL(lexer));
//   while (_lexer_tokenize_line(lexer))
//     ;
// }
