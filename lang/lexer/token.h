#ifndef LANGUAGE_TOOLS_LANG_LEXER_TOKEN_H_
#define LANGUAGE_TOOLS_LANG_LEXER_TOKEN_H_

#include <stdlib.h>

typedef struct {
  int type;
  int col, line;
  size_t len;
  const char *text;
} Token;

Token *token_create(int type, int line, int col, const char text[],
                    int text_len);
void token_fill(Token *tok, int type, int line, int col, const char text[],
                int text_len);
void token_delete(Token *tok);
void token_finalize_all();

#endif /* LANGUAGE_TOOLS_LANG_LEXER_TOKEN_H_ */