#ifndef LANGUAGE_TOOLS_LANG_LEXER_LEXER_BUILDER_H_
#define LANGUAGE_TOOLS_LANG_LEXER_LEXER_BUILDER_H_

#include <stdio.h>

#include "util/file/file_info.h"

typedef struct _LexerBuilder LexerBuilder;

LexerBuilder *lexer_builder_create(FileInfo *symbols, FileInfo *keywords,
                                   FileInfo *comments, FileInfo *strings);
void lexer_builder_write_c_file(LexerBuilder *lb, FILE *file,
                                const char h_file_path[]);
void lexer_builder_write_h_file(LexerBuilder *lb, FILE *file);
void lexer_builder_delete(LexerBuilder *lb);

#endif /* LANGUAGE_TOOLS_LANG_LEXER_LEXER_BUILDER_H_ */