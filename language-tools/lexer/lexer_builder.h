#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_BUILDER_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_BUILDER_H_

#include <stdio.h>

#include "c-data-structures/arraylike.h"
#include "file-utils/file_info.h"

#define MAX_CHAR 256

typedef struct {
  const char *token;
  const char *escaped_token;
  const char *token_name;
  int token_len;
} TokenDef_;

DEFINE_ARRAYLIKE(TokenDefArray, TokenDef_);

typedef struct {
  const char *token_name;
  TokenDef_ open;
  TokenDef_ close;
} OpenCloseDef_;

DEFINE_ARRAYLIKE(OpenCloseDefArray, OpenCloseDef_);

typedef struct Trie__ Trie_;

struct Trie__ {
  Trie_ *chars[MAX_CHAR];
  TokenDef_ *has;
};

typedef struct {
  TokenDefArray symbols;
  TokenDefArray keywords;
  Trie_ *symbols_trie;
  Trie_ *keywords_trie;
  OpenCloseDefArray comments;
  OpenCloseDefArray strings;
} LexerBuilder;

void lexer_builder_init(LexerBuilder *lb, FileInfo *symbols, FileInfo *keywords,
                        FileInfo *comments, FileInfo *strings);
void lexer_builder_write_c_file(LexerBuilder *lb, FILE *file,
                                const char h_file_path[],
                                const char fn_prefix[],
                                const char enum_prefix[]);
void lexer_builder_write_h_file(LexerBuilder *lb, FILE *file,
                                const char fn_prefix[],
                                const char enum_prefix[]);
void lexer_builder_finalize(LexerBuilder *lb);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_BUILDER_H_ */