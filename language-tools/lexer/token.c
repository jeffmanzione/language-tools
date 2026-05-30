#include "language-tools/lexer/token.h"

#include <string.h>

#include "language-tools/intern.h"
#include "rzalloc/rzalloc.h"

IMPL_ARRAYLIKE(TokenArray, Token *);

static RzallocArena token_arena_;

static bool inited = false;

void token_fill(Token *tok, int type, int line, int col, const char text[],
                int text_len) {
  tok->type = type;
  tok->line = line;
  tok->col = col;
  tok->len = text_len;
  tok->text = global_intern_range(text, 0, text_len);
}

Token *token_create(int type, int line, int col, const char text[],
                    int text_len) {
  if (!inited) {
    arena_init(&token_arena_, sizeof(Token));
    inited = true;
  }
  Token *tok = (Token *)arena_malloc(&token_arena_);
  token_fill(tok, type, line, col, text, text_len);
  return tok;
}

void token_delete(Token *token) { arena_free(&token_arena_, token); }

void token_finalize_all() {
  if (!inited) {
    return;
  }
  arena_clear(&token_arena_);
}