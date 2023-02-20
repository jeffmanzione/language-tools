#include "lang/lexer/token.h"

#include <string.h>

#include "alloc/arena/arena.h"
#include "alloc/arena/intern.h"
#include "debug/debug.h"

static ARENA_DEFINE(Token);

static bool inited = false;

void token_fill(Token *tok, int type, int line, int col, const char text[],
                int text_len) {
  tok->type = type;
  tok->line = line;
  tok->col = col;
  tok->len = text_len;
  tok->text = intern_range(text, 0, text_len);
}

Token *token_create(int type, int line, int col, const char text[],
                    int text_len) {
  if (!inited) {
    ARENA_INIT(Token);
    inited = true;
  }
  Token *tok = ARENA_ALLOC(Token);
  token_fill(tok, type, line, col, text, text_len);
  return tok;
}

void token_delete(Token *token) {
  ASSERT_NOT_NULL(token);
  ARENA_DEALLOC(Token, token);
}

void token_finalize_all() {
  if (!inited) {
    return;
  }
  ARENA_FINALIZE(Token);
}