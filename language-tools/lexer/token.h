#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_TOKEN_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_TOKEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "c-data-structures/arraylike.h"

typedef struct {
  int type;
  int col, line;
  size_t len;
  const char *text;
} Token;

DEFINE_ARRAYLIKE(TokenArray, Token *);

Token *token_create(int type, int line, int col, const char text[],
                    int text_len);
void token_fill(Token *tok, int type, int line, int col, const char text[],
                int text_len);
void token_delete(Token *tok);
void token_finalize_all();

#ifdef __cplusplus
}
#endif

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_TOKEN_H_ */