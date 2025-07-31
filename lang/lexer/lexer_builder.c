#include "lang/lexer/lexer_builder.h"

#include <stdio.h>

#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "lang/lexer/lexer_helper.h"
#include "struct/alist.h"
#include "util/file/file_info.h"
#include "util/string.h"

#define MAX_CHAR 256

typedef struct {
  char *token;
  char *token_name;
  int token_len;
} _TokenDef;

typedef struct {
  char *token_name;
  _TokenDef open;
  _TokenDef close;
} _OpenCloseDef;

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
  AList strings;
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

char *_escape_interned(const char *str) {
  char *tmp = escape_string(str);
  char *interned = intern(tmp);
  DEALLOC(tmp);
  return interned;
}

void _build_token_list(FileInfo *file, AList *tokens) {
  LineInfo *li;
  alist_init(tokens, _TokenDef, DEFAULT_ARRAY_SZ);
  while (NULL != (li = file_info_getline(file))) {
    char *comma =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    uint32_t comma_index = comma - li->line_text;
    _TokenDef *def = alist_add(tokens);
    char *token_unescaped = intern_range(comma, 1, strlen(comma + 1));
    def->token = _escape_interned(token_unescaped);
    def->token_len = strlen(token_unescaped);
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

void _build_open_close_list(FileInfo *file, AList *tokens) {
  LineInfo *li;
  alist_init(tokens, _OpenCloseDef, DEFAULT_ARRAY_SZ);
  while (NULL != (li = file_info_getline(file))) {
    char *comma1 =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    char *comma2 = find_str(comma1 + 1, strlen(comma1 + 1), ",", strlen(","));

    _OpenCloseDef *def = alist_add(tokens);
    def->token_name = intern_range(li->line_text, 0, comma1 - li->line_text);

    char *token_unesc = intern_range(comma1, 1, comma2 - comma1);
    char *open = strip_return_char(token_unesc);
    def->open.token = _escape_interned(open);
    def->open.token_len = strlen(open);
    DEALLOC(open);

    token_unesc = intern_range(comma2, 1, strlen(comma2 + 1));
    char *close = strip_return_char(token_unesc);
    def->close.token = _escape_interned(close);
    def->close.token_len = strlen(close);
    DEALLOC(close);
    def->open.token_name =
        _string_copy_and_append(li->line_text, comma1 - li->line_text, "_OPEN");
    def->close.token_name = _string_copy_and_append(
        li->line_text, comma1 - li->line_text, "_CLOSE");
  }
}

LexerBuilder *lexer_builder_create(FileInfo *symbols, FileInfo *keywords,
                                   FileInfo *comments, FileInfo *strings) {
  LexerBuilder *lb = ALLOC2(LexerBuilder);
  _build_token_list(symbols, &lb->symbols);
  _build_token_list(keywords, &lb->keywords);
  _build_open_close_list(comments, &lb->comments);
  _build_open_close_list(strings, &lb->strings);
  lb->symbols_trie = _create_trie_from_symbols(&lb->symbols);
  lb->keywords_trie = _create_trie_from_symbols(&lb->keywords);
  return lb;
}

void _write_header(LexerBuilder *lb, FILE *file, const char h_file_path[]) {
  // Includes.
  fprintf(file, "#include \"%s\"\n\n", h_file_path);
  fprintf(file, "#include \"lang/lexer/lexer_helper.h\"\n");
  fprintf(file, "#include \"lang/lexer/token.h\"\n");
  fprintf(file, "#include \"util/string.h\"\n");
  fprintf(file, "\n");
}

void _write_token_type_enum(LexerBuilder *lb, FILE *file) {
  fprintf(file,
          "typedef enum {\n"
          "  TOKENTYPE_UNKNOWN,\n"
          "  TOKEN_NEWLINE,\n"  // Must stay at index 1 for parser.
          "  TOKEN_WORD,\n"
          "  TOKEN_INTEGER,\n"
          "  TOKEN_FLOATING,\n");
  AL_iter iter = alist_iter(&lb->strings);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *open_close_def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "  %s,\n", open_close_def->token_name);
  }
  iter = alist_iter(&lb->symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  iter = alist_iter(&lb->keywords);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  fprintf(file, "  TOKEN_NOP\n");
  fprintf(file, "} LexType;\n\n");
}

void _write_token_type_to_str(LexerBuilder *lb, FILE *file) {
  fprintf(file, "const char *token_type_to_str(LexType token_type) {\n");
  fprintf(file, "  switch(token_type) {\n");
  fprintf(file, "    case TOKEN_NEWLINE: return \"\\n\";\n");
  fprintf(file, "    case TOKEN_WORD: return \"TOKEN_WORD\";\n");
  fprintf(file, "    case TOKEN_INTEGER: return \"TOKEN_INTEGER\";\n");
  fprintf(file, "    case TOKEN_FLOATING: return \"TOKEN_FLOATING\";\n");
  AL_iter iter = alist_iter(&lb->strings);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *open_close_def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", open_close_def->token_name,
            open_close_def->token_name);
  }
  iter = alist_iter(&lb->symbols);
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
  fprintf(file, "    default: return \"UNKNOWN\";\n  }\n}\n\n");
}

void _write_token_name_to_token_type(LexerBuilder *lb, FILE *file) {
  fprintf(file, "LexType token_name_to_token_type(const char str[]) {\n");
  fprintf(file,
          "  if (0 == strcmp(\"TOKEN_NEWLINE\", str)) return TOKEN_NEWLINE;\n");
  fprintf(file, "  if (0 == strcmp(\"TOKEN_WORD\", str)) return TOKEN_WORD;\n");
  fprintf(file,
          "  if (0 == strcmp(\"TOKEN_INTEGER\", str)) return TOKEN_INTEGER;\n");
  fprintf(
      file,
      "  if (0 == strcmp(\"TOKEN_FLOATING\", str)) return TOKEN_FLOATING;\n");
  AL_iter iter = alist_iter(&lb->strings);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *open_close_def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            open_close_def->token_name, open_close_def->token_name);
  }
  iter = alist_iter(&lb->symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            token_def->token_name, token_def->token_name);
  }
  iter = alist_iter(&lb->keywords);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            token_def->token_name, token_def->token_name);
  }
  fprintf(file, "    return 0;\n}\n\n");
}

void _write_token_type_to_name(LexerBuilder *lb, FILE *file) {
  fprintf(file, "const char *token_type_to_name(LexType token_type) {\n");
  fprintf(file, "  switch(token_type) {\n");
  fprintf(file, "    case TOKEN_NEWLINE: return \"TOKEN_NEWLINE\";\n");
  fprintf(file, "    case TOKEN_WORD: return \"TOKEN_WORD\";\n");
  fprintf(file, "    case TOKEN_INTEGER: return \"TOKEN_INTEGER\";\n");
  fprintf(file, "    case TOKEN_FLOATING: return \"TOKEN_FLOATING\";\n");
  AL_iter iter = alist_iter(&lb->strings);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *open_close_def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", open_close_def->token_name,
            open_close_def->token_name);
  }
  iter = alist_iter(&lb->symbols);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token_name);
  }
  iter = alist_iter(&lb->keywords);
  for (; al_has(&iter); al_inc(&iter)) {
    _TokenDef *token_def = (_TokenDef *)al_value(&iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token_name);
  }
  fprintf(file, "    default: return \"UNKNOWN\";\n  }\n}\n\n");
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
      if (((char)i) == '\\') {
        fprintf(file, "%*scase '\\\\':\n", index * 2, "");
      } else {
        fprintf(file, "%*scase '%c':\n", index * 2, "", (char)i);
      }
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
  fprintf(file, "LexType symbol_token_type(const char word[]) {\n");
  _write_switch_for_symbol_resolve(lb->symbols_trie, 1, file);
  fprintf(file, "  return TOKENTYPE_UNKNOWN;\n");
  fprintf(file, "}\n\n");

  fprintf(file, "LexType _keyword_type(const char word[], int word_len) {\n");
  fprintf(file, "  if (word_len <= 0) { return TOKEN_NEWLINE; }\n");
  _write_switch_for_keyword_resolve(lb->keywords_trie, 1, file);
  fprintf(file, "}\n\n");

  fprintf(file, "LexType resolve_type(const char word[], int word_len) {\n");
  fprintf(file, "  LexType type = symbol_token_type(word);\n");
  fprintf(file,
          "  if (TOKENTYPE_UNKNOWN == type) { type = _keyword_type(word, "
          "word_len); }\n");
  fprintf(file, "  if (TOKENTYPE_UNKNOWN != type) { return type; }\n");
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
    if (((char)i) == '\\') {
      fprintf(file, "    case '\\\\':\n");
    } else {
      fprintf(file, "    case '%c':\n", (char)i);
    }
  }
  fprintf(file, "      return true;\n");
  fprintf(file, "    default: return false;\n");
  fprintf(file, "  }\n}\n\n");
}

void _write_token_type_is_string(LexerBuilder *lb, FILE *file) {
  fprintf(file, "bool token_type_is_string(LexType type) {\n");
  AL_iter iter = alist_iter(&lb->strings);
  fprintf(file, "  switch (type) {\n");
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *open_close_def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "    case %s:\n", open_close_def->token_name);
  }
  fprintf(file, "      return true;\n");
  fprintf(file, "    default:\n");
  fprintf(file, "      return false;\n");
  fprintf(file, "  }\n}\n\n");
}

void _write_is_start_of_open_close(AList *list, const char fn_name[],
                                   FILE *file) {
  fprintf(file, "const char *%s(const char word[]) {\n", fn_name);
  AL_iter iter = alist_iter(list);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "  if (0 == strncmp(\"%s\", word, %d)) {\n", def->open.token,
            def->open.token_len);
    fprintf(file, "    return \"%s\";\n  }\n", def->close.token);
  }
  fprintf(file, "  return NULL;\n}\n\n");
}

void _write_is_start_string(LexerBuilder *lb, FILE *file) {
  fprintf(file,
          "bool is_start_of_string(const char word[], LexType *string_type, "
          "int *string_open_len, char **string_close) {\n");
  AL_iter iter = alist_iter(&lb->strings);
  for (; al_has(&iter); al_inc(&iter)) {
    _OpenCloseDef *def = (_OpenCloseDef *)al_value(&iter);
    fprintf(file, "  if (0 == strncmp(\"%s\", word, %d)) {\n", def->open.token,
            def->open.token_len);
    fprintf(file, "    *string_type = %s;\n", def->token_name);
    fprintf(file, "    *string_open_len = %d;\n", def->open.token_len);
    fprintf(file, "    *string_close = \"%s\";\n", def->close.token);
    fprintf(file, "    return true;\n  }\n");
  }
  fprintf(file, "  return false;\n}\n\n");
}

const char _TOKENIZE_FUNCTIONS_TEXT[] =
    "\n\
int _tokenize_number(const LineInfo *li, Q *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  int start = col_num;\n\
  bool is_decimal = false;\n\
  while (is_number(line[col_num])) {\n\
    if ('.' == line[col_num]) {\n\
      // Decimals cannot have more than 1 decimal point.\n\
      if (is_decimal) {\n\
        break;\n\
      } else {\n\
        is_decimal = true;\n\
      }\n\
    }\n\
    ++col_num;\n\
  }\n\
  if ('f' == line[col_num]) {\n\
    is_decimal = true;\n\
    ++col_num;\n\
  }\n\
  Token *token =\n\
      token_create(is_decimal ? TOKEN_FLOATING : TOKEN_INTEGER, li->line_num,\n\
                   start, line + start, col_num - start);\n\
  *Q_add_last(tokens) = token;\n\
  return col_num;\n\
}\n\
\n\
int _tokenize_symbol(const LineInfo *li, Q *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  LexType type = symbol_token_type(line + col_num);\n\
  if (TOKENTYPE_UNKNOWN == type) {\n\
    FATALF(\"UNKNOWN TOKEN\");\n\
  }\n\
  const int token_length = strlen(token_type_to_str(type));\n\
  Token *token =\n\
      token_create(type, li->line_num, col_num, line + col_num, token_length);\n\
  *Q_add_last(tokens) = token;\n\
  col_num += token_length;\n\
  return col_num;\n\
}\n\
\n\
int _tokenize_word(const LineInfo *li, Q *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  int start = col_num++;\n\
  while (is_alphanumeric(line[col_num])) {\n\
    ++col_num;\n\
  }\n\
  LexType token_type = _keyword_type(line + start, col_num - start);\n\
  Token *token = token_create(\n\
      token_type == TOKENTYPE_UNKNOWN ? TOKEN_WORD : token_type,\n\
      li->line_num,\n\
      start,\n\
      line + start,\n\
      col_num - start);\n\
  *Q_add_last(tokens) = token;\n\
  return col_num;\n\
}\n\
\n\
int _tokenize_newline(const LineInfo *li, Q *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  Token *last = Q_is_empty(tokens) ? NULL : (Token *) Q_get(tokens, Q_size(tokens) - 1);\n\
  if (NULL == last || last->type != TOKEN_NEWLINE) {\n\
    Token *token =\n\
        token_create(TOKEN_NEWLINE, li->line_num, col_num, line + col_num, 1);\n\
    *Q_add_last(tokens) = token;\n\
  }\n\
  ++col_num;\n\
  return col_num;\n\
}\n\
\n\
bool _lexer_tokenize_line(FileInfo *fi, Q *tokens, bool *in_comment, bool *in_string,\n\
                          const char **comment_end, const char **string_end, LexType *string_type, char **string_buffer) {\n\
  LineInfo *li = file_info_getline(fi);\n\
  if (NULL == li) {\n\
    return false;\n\
  }\n\
  printf(\">> line %zd '%s'\\n\", strlen(li->line_text), li->line_text);\n\
  int col_num = 0;\n\
  int string_start_col = 0;\n\
  char *line = li->line_text;\n\
  while (true) {\n\
    if ('\\0' == line[col_num]) {\n\
      break;\n\
    }\n\
    while (is_whitespace(line[col_num])) {\n\
      ++col_num;\n\
    }\n\
    if (*in_comment) {\n\
      char *eoc = find_str(line + col_num,\n\
                            strlen(line) - col_num, *comment_end, \n\
                            strlen(*comment_end)); \n\
      // End of comment not found.\n\
      if (NULL == eoc) {\n\
        return true;\n\
      }\n\
      col_num = eoc - line + strlen(*comment_end);\n\
      // Preserve newline if it is the last character.\n\
      if (ends_with(*comment_end, \"\\n\")) {\n\
        col_num--;\n\
      }\n\
      *in_comment = false;\n\
      *comment_end = NULL;\n\
      continue;\n\
    }\n\
    if (*in_string) {\n\
      char *eos = line + col_num - 1;\n\
      while (true) {\n\
        eos = find_str(eos + 1,\n\
                       strlen(line) - (eos - line + 1), *string_end, \n\
                       strlen(*string_end)); \n\
        if (NULL == eos ||  *(eos - 1) != '\\\\') {\n\
          break;\n\
        }\n\
      }\n\
      if (NULL == eos) {\n\
        eos = line + strlen(line);\n\
      }\n\
      col_num = eos - line;\n\
      if (NULL == *string_buffer) {\n\
        *string_buffer = ALLOC_ARRAY2(char, col_num - string_start_col + 1);\n\
        memmove(*string_buffer, line + string_start_col, col_num - string_start_col);\n\
        (*string_buffer)[col_num - string_start_col] = '\\0';\n\
      } else {\n\
        char *new_string_buffer =\n\
            ALLOC_ARRAY2(char, strlen(*string_buffer) + col_num - string_start_col + 1);\n\
        memmove(new_string_buffer, *string_buffer, strlen(*string_buffer));\n\
        memmove(new_string_buffer + strlen(*string_buffer),  line + string_start_col, col_num - string_start_col);\n\
        new_string_buffer[strlen(*string_buffer) + col_num - string_start_col] = '\\0';\n\
        DEALLOC(*string_buffer);\n\
        *string_buffer = new_string_buffer;\n\
      }\n\
      // End of string not found.\n\
      if (line + strlen(line) == eos) {\n\
        return true;\n\
      }\n\
      Token *token = token_create(*string_type,\n\
                   li->line_num, string_start_col, *string_buffer, strlen(*string_buffer));\n\
      DEALLOC(*string_buffer);\n\
      *string_buffer = NULL;\n\
      *Q_add_last(tokens) = token;\n\
      col_num = eos - line + strlen(*string_end);\n\
      *in_string = false;\n\
      *string_end = NULL;\n\
      continue;\n\
    }\n\
    const char *eoc = is_start_of_comment(line + col_num);\n\
    if (NULL != eoc) {\n\
      *in_comment = true;\n\
      *comment_end = eoc;\n\
      col_num++;\n\
      continue;\n\
    }\n\
    int string_open_len;\n\
    if (is_start_of_string(line + col_num, string_type, &string_open_len, string_end)) {\n\
      *in_string = true;\n\
      printf(\"%s %d\\n\", *string_end, string_open_len);\n\
      col_num += string_open_len;\n\
      string_start_col = col_num;\n\
      continue;\n\
    }\n\
    if ('\\0' == line[col_num]) {\n\
      continue;\n\
    } else if (is_numeric(line[col_num])) {\n\
      col_num = _tokenize_number(li, tokens, col_num);\n\
    } else if (is_start_of_symbol(line + col_num)) {\n\
      col_num = _tokenize_symbol(li, tokens, col_num);\n\
    } else if (is_alphanumeric(line[col_num])) {\n\
      col_num = _tokenize_word(li, tokens, col_num);\n\
    } else if ('\\n' == line[col_num] || '\\r' == line[col_num]) {\n\
      col_num = _tokenize_newline(li, tokens, col_num);\n\
    } else {\n\
      printf(\"%d:%d \\\"%c\\\"\\n\", li->line_num, col_num, line[col_num]);\n\
      printf(\"line: \\\"%s\\\"\\n\", line);\n\
      fflush(stdout);\n\
      FATALF(\"NEVER HERE!\");\n\
    }\n\
  }\n\
  return true;\n\
}\n\
\n\
void lexer_tokenize_line(FileInfo *file, Q *tokens) {\n\
  ASSERT(NOT_NULL(file), NOT_NULL(tokens));\n\
  bool in_comment = false;\n\
  const char *comment_end = NULL;\n\
  bool in_string = false;\n\
  const char *string_end = NULL;\n\
  LexType string_type = TOKENTYPE_UNKNOWN;\n\
  char *string_buffer = NULL;\n\
  _lexer_tokenize_line(file, tokens, &in_comment, &in_string, &comment_end, &string_end, &string_type, &string_buffer);\n\
}\n\
void lexer_tokenize(FileInfo *file, Q *tokens) {\n\
  ASSERT(NOT_NULL(file), NOT_NULL(tokens));\n\
  bool in_comment = false;\n\
  const char *comment_end = NULL;\n\
  bool in_string = false;\n\
  const char *string_end = NULL;\n\
  LexType string_type;\n\
  char *string_buffer = NULL;\n\
  while (_lexer_tokenize_line(file, tokens, &in_comment, &in_string, &comment_end, &string_end, &string_type, &string_buffer))\n\
    ;\n\
}";

void lexer_builder_write_c_file(LexerBuilder *lb, FILE *file,
                                const char h_file_path[]) {
  _write_header(lb, file, h_file_path);
  _write_token_type_to_str(lb, file);
  _write_token_type_to_name(lb, file);
  _write_token_name_to_token_type(lb, file);
  _write_resolve_type(lb, file);
  _write_is_start_of_symbol(lb, file);
  _write_is_start_of_open_close(&lb->comments, "is_start_of_comment", file);
  _write_is_start_string(lb, file);
  _write_token_type_is_string(lb, file);
  fprintf(file, "%s\n", _TOKENIZE_FUNCTIONS_TEXT);
}

void lexer_builder_write_h_file(LexerBuilder *lb, FILE *file) {
  fprintf(file, "#ifndef LANG_LEXER_CUSTOM_LEXER_H_\n");
  fprintf(file, "#define LANG_LEXER_CUSTOM_LEXER_H_\n\n");
  fprintf(file, "#include <stdbool.h>\n\n");
  fprintf(file, "#include \"util/file/file_info.h\"\n");
  fprintf(file, "#include \"struct/q.h\"\n\n");
  _write_token_type_enum(lb, file);
  fprintf(file, "LexType symbol_token_type(const char word[]);\n");
  fprintf(file, "const char *token_type_to_str(LexType token_type);\n");
  fprintf(file, "const char *token_type_to_name(LexType token_type);\n");
  fprintf(file, "LexType token_name_to_token_type(const char str[]);\n");
  fprintf(file, "LexType resolve_type(const char word[], int word_len);\n");
  fprintf(file, "bool is_start_of_symbol(const char word[]);\n");
  fprintf(file, "const char *is_start_of_comment(const char word[]);\n");
  fprintf(file,
          "bool is_start_of_string(const char word[], LexType *string_type, "
          "int *string_open_len, char **string_close);\n");
  fprintf(file, "bool token_type_is_string(LexType type);\n");
  fprintf(file, "void lexer_tokenize_line(FileInfo *file, Q *tokens);\n");
  fprintf(file, "void lexer_tokenize(FileInfo *file, Q *tokens);\n");
  fprintf(file, "\n#endif /* LANG_LEXER_CUSTOM_LEXER_H_ */\n");
}

void lexer_builder_delete(LexerBuilder *lb) {
  alist_finalize(&lb->symbols);
  alist_finalize(&lb->keywords);
  _trie_delete(lb->symbols_trie);
  _trie_delete(lb->keywords_trie);
  alist_finalize(&lb->comments);
  DEALLOC(lb);
}
