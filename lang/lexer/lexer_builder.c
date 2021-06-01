#include "lang/lexer/lexer_builder.h"

#include <stdio.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "struct/alist.h"
#include "util/file/file_info.h"
#include "util/string.h"

#define MAX_CHAR 256

typedef struct {
  char *token;
  char *token_name;
} _TokenDef;

typedef struct {
  _TokenDef open;
  _TokenDef close;
} _CommentDef;

typedef struct __Trie _Trie;

struct __Trie {
  _Trie *chars[MAX_CHAR];
  _TokenDef *has;
};

struct _LexerBuilder {
  AList symbols;
  AList keywords;
  _Trie *symbols_trie;
  _Trie *keywords_trie;
  AList comments;
};

_Trie *_trie_create() {
  // DO NOT CHANGE TO ALLOC2.
  _Trie *trie = ALLOC(_Trie);
  return trie;
}

void _trie_delete(_Trie *trie) {
  int i;
  for (i = 0; i < MAX_CHAR; ++i) {
    _Trie *child = trie->chars[i];
    if (NULL != child) {
      _trie_delete(child);
    }
  }
  DEALLOC(trie);
}

bool _has_child_trie(_Trie *trie) {
  int i;
  for (i = 0; i < MAX_CHAR; ++i) {
    _Trie *child = trie->chars[i];
    if (NULL != child) {
      return true;
    }
  }
  return false;
}

_Trie *_create_trie_from_symbols(AList *symbols) {
  _Trie *base = _trie_create();
  AL_iter iter = alist_iter(symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    int i;
    _Trie *cur = base;
    for (i = 0; i < strlen(token_def->token); ++i) {
      if (NULL == cur->chars[(uint32_t)token_def->token[i]]) {
        cur->chars[(uint32_t)token_def->token[i]] = _trie_create();
      }
      cur = cur->chars[(uint32_t)token_def->token[i]];
    }
    cur->has = token_def;
  }
  return base;
}

void _build_token_list(FileInfo *file, AList *tokens) {
  LineInfo *li;
  alist_init(tokens, _TokenDef, DEFAULT_ARRAY_SZ);
  while (NULL != (li = file_info_getline(file))) {
    char *comma =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    uint32_t comma_index = comma - li->line_text;
    _TokenDef *def = alist_add(tokens);
    def->token =
        intern_range(comma, 1,
                     ends_with(comma + 1, "\n") ? (strlen(comma + 1) - 1)
                                                : strlen(comma + 1));
    def->token_name = intern_range(li->line_text, 0, comma_index);
  }
}

char *_string_copy_and_append(const char main[], size_t main_len,
                              const char suffix[]) {
  char *tmp = ALLOC_ARRAY(char, main_len + strlen(suffix) + 1);
  strncpy(tmp, main, main_len);
  strcpy(tmp + main_len, suffix);
  char *final = intern(tmp);
  DEALLOC(tmp);
  return final;
}

void _build_comment_list(FileInfo *file, AList *tokens) {
  LineInfo *li;
  alist_init(tokens, _CommentDef, DEFAULT_ARRAY_SZ);
  while (NULL != (li = file_info_getline(file))) {
    char *comma1 =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    char *comma2 = find_str(comma1 + 1, strlen(comma1 + 1), ",", strlen(","));
    _CommentDef *def = alist_add(tokens);
    def->open.token = intern_range(comma1, 1, comma2 - comma1);
    def->close.token =
        intern_range(comma2, 1,
                     ends_with(comma2 + 1, "\n") ? (strlen(comma2 + 1) - 1)
                                                 : strlen(comma2 + 1));
    def->open.token_name =
        _string_copy_and_append(li->line_text, comma1 - li->line_text, "_OPEN");
    def->close.token_name = _string_copy_and_append(
        li->line_text, comma1 - li->line_text, "_CLOSE");
  }
}

LexerBuilder *lexer_builder_create(FileInfo *symbols, FileInfo *keywords,
                                   FileInfo *comments) {
  LexerBuilder *lb = ALLOC2(LexerBuilder);
  _build_token_list(symbols, &lb->symbols);
  _build_token_list(keywords, &lb->keywords);
  _build_comment_list(comments, &lb->comments);
  lb->symbols_trie = _create_trie_from_symbols(&lb->symbols);
  lb->keywords_trie = _create_trie_from_symbols(&lb->keywords);
  return lb;
}

void _write_header(LexerBuilder *lb, FILE *file) {
  // Includes.
  fprintf(file, "#include <stdbool.h>\n\n");
  fprintf(file, "#include \"lang/lexer/lexer_helper.h\"\n");
  fprintf(file, "\n");
}

void _write_token_type_enum(LexerBuilder *lb, FILE *file) {
  fprintf(file, "typedef enum {\n"
                "  TOKENTYPE_UNKNOWN,\n"
                "  TOKEN_WORD,\n"
                "  TOKEN_STRING\n"
                "  TOKEN_INTEGER\n"
                "  TOKEN_FLOATING\n");
  AL_iter iter = alist_iter(&lb->symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  iter = alist_iter(&lb->keywords);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  fprintf(file, "  ENDLINE\n");
  fprintf(file, "} TokenType;\n\n");
}

void _write_token_type_to_str(LexerBuilder *lb, FILE *file) {
  fprintf(file, "const char *token_type_to_str(TokenType token_type) {\n");
  fprintf(file, "  switch(token_type) {\n");
  AL_iter iter = alist_iter(&lb->symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token);
  }
  iter = alist_iter(&lb->keywords);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token);
  }
  fprintf(file, "    case ENDLINE: return \"\\n\";\n");
  fprintf(file, "    default: \"UNKNOWN\";\n  }\n}\n\n");
}

void _write_switch_for_symbol_resolve(_Trie *trie, int index, FILE *file) {
  int i;
  const bool has_children = _has_child_trie(trie);
  if (has_children) {
    fprintf(file, "%*sswitch (word[%d]) {\n", index * 2, "", index - 1);
    for (i = 0; i < MAX_CHAR; ++i) {
      _Trie *child = trie->chars[i];
      if (NULL == child) {
        continue;
      }
      fprintf(file, "%*scase '%c':\n", index * 2, "", (char)i);
      _write_switch_for_symbol_resolve(child, index + 1, file);
    }
    fprintf(file, "%*sdefault: break;\n", index * 2, "");
    fprintf(file, "%*s}\n", index * 2, "");
  }
  if (trie->has) {
    fprintf(file, "%*sreturn %s;\n", index * 2, "", trie->has->token_name);
  }
}

void _write_switch_for_keyword_resolve(_Trie *trie, int index, FILE *file) {
  int i;
  const bool has_children = _has_child_trie(trie);
  if (trie->has) {
    fprintf(file, "%*sif (word_len == %d) { return %s; }\n", index * 2, "",
            index - 1, trie->has->token_name);
  }
  if (has_children) {
    fprintf(file, "%*sswitch (word[%d]) {\n", index * 2, "", index - 1);
    for (i = 0; i < MAX_CHAR; ++i) {
      _Trie *child = trie->chars[i];
      if (NULL == child) {
        continue;
      }
      fprintf(file, "%*scase '%c':\n", index * 2, "", (char)i);
      _write_switch_for_keyword_resolve(child, index + 1, file);
    }
    fprintf(file, "%*sdefault: return TOKENTYPE_UNKNOWN;\n", index * 2, "");
    fprintf(file, "%*s}\n", index * 2, "");
  }
  fprintf(file, "%*sreturn TOKENTYPE_UNKNOWN;\n", index * 2, "");
}

void _write_resolve_type(LexerBuilder *lb, FILE *file) {
  fprintf(file, "TokenType _symbol_type(const char word[]) {\n");
  _write_switch_for_symbol_resolve(lb->symbols_trie, 1, file);
  fprintf(file, "}\n\n");

  fprintf(file, "TokenType _keyword_type(const char word[], int word_len) {\n");
  fprintf(file, "  if (word_len <= 0) { return ENDLINE; }\n");
  _write_switch_for_keyword_resolve(lb->keywords_trie, 1, file);
  fprintf(file, "}\n\n");

  fprintf(file, "TokenType resolve_type(const char word[], int world_len) {\n");
  fprintf(file, "  TokenType type = _symbol_type(word);\n");
  fprintf(file, "  if (TOKENTYPE_UNKNOWN == type) { type = _keyword_type(word, "
                "word_len); }\n");
  fprintf(file,
          "  if (is_number(word[0])) {\n"
          "    if (ends_with(word, \"f\") || contains_char(word, '.')) {\n"
          "      return TOKEN_FLOATING;\n"
          "    } else { return TOKEN_INTEGER; }\n"
          "  }\n");
  fprintf(file, "  return type;\n}\n\n");
}

void _write_is_start_of_symbol(LexerBuilder *lb, FILE *file) {
  fprintf(file, "bool is_start_of_symbol(const char word[]) {\n");
  int i;
  fprintf(file, "  switch (word[0]) {\n");
  for (i = 0; i < MAX_CHAR; ++i) {
    _Trie *child = lb->symbols_trie->chars[i];
    if (NULL == child) {
      continue;
    }
    fprintf(file, "    case '%c':\n", (char)i);
  }
  fprintf(file, "      return true;\n");
  fprintf(file, "    default: return false;\n");
  fprintf(file, "  }\n}\n\n");
}

void lexer_builder_write_c_file(LexerBuilder *lb, FILE *file) {
  _write_header(lb, file);
  _write_token_type_enum(lb, file);
  _write_token_type_to_str(lb, file);
  _write_resolve_type(lb, file);
  _write_is_start_of_symbol(lb, file);
}

void lexer_builder_delete(LexerBuilder *lb) {
  alist_finalize(&lb->symbols);
  alist_finalize(&lb->keywords);
  _trie_delete(lb->symbols_trie);
  _trie_delete(lb->keywords_trie);
  alist_finalize(&lb->comments);
  DEALLOC(lb);
}
