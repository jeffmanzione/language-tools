#include "language-tools/lexer/lexer_builder.h"

#include <stdio.h>

#include "file-utils/file_info.h"
#include "file-utils/string_utils.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/lexer_helper.h"

IMPL_ARRAYLIKE(TokenDefArray, TokenDef_);
IMPL_ARRAYLIKE(OpenCloseDefArray, OpenCloseDef_);

Trie_ *trie_create_() {
  Trie_ *trie = calloc(1, sizeof(Trie_));
  return trie;
}

void trie_delete_(Trie_ *trie) {
  int i;
  for (i = 0; i < TRIE_MAX_CHAR_; ++i) {
    Trie_ *child = trie->chars[i];
    if (NULL != child) {
      trie_delete_(child);
    }
  }
  free(trie);
}

bool has_child_trie_(Trie_ *trie) {
  int i;
  for (i = 0; i < TRIE_MAX_CHAR_; ++i) {
    Trie_ *child = trie->chars[i];
    if (NULL != child) {
      return true;
    }
  }
  return false;
}

Trie_ *create_trie_from_symbols_(TokenDefArray *symbols) {
  Trie_ *base = trie_create_();
  TokenDefArrayIterator iter;
  TokenDefArray_iterator(&iter, symbols);
  for (; TokenDefArray_has_next(&iter); TokenDefArray_next(&iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&iter);
    int i;
    Trie_ *cur = base;
    for (i = 0; i < strlen(token_def->escaped_token); ++i) {
      if (NULL == cur->chars[(uint32_t)token_def->escaped_token[i]]) {
        cur->chars[(uint32_t)token_def->escaped_token[i]] = trie_create_();
      }
      cur = cur->chars[(uint32_t)token_def->escaped_token[i]];
    }
    cur->has = token_def;
  }
  return base;
}

const char *escape_interned_(const char *str) {
  char *tmp = escape_string(str);
  const char *interned = global_intern(tmp);
  free(tmp);
  return interned;
}

void build_token_list_(FileInfo *file, TokenDefArray *tokens) {
  LineInfo *li;
  TokenDefArray_init(tokens);
  while (NULL != (li = file_info_getline(file))) {
    char *comma =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    uint32_t comma_index = comma - li->line_text;
    TokenDef_ *def = TokenDefArray_push_back_ref(tokens);
    const char *token_unescaped =
        global_intern_range(comma, 1, strlen(comma + 1) - 1);
    def->escaped_token = escape_interned_(token_unescaped);
    def->token_len = strlen(token_unescaped);
    def->token_name = global_intern_range(li->line_text, 0, comma_index);
  }
}

const char *string_copy_and_append_(const char main[], size_t main_len,
                                    const char suffix[]) {
  char *tmp = calloc((main_len + strlen(suffix) + 1), sizeof(char));
  strncpy(tmp, main, main_len);
  strcpy(tmp + main_len, suffix);
  const char *final = global_intern(tmp);
  free(tmp);
  return final;
}

void build_open_close_list_(FileInfo *file, OpenCloseDefArray *tokens) {
  LineInfo *li;
  OpenCloseDefArray_init(tokens);
  while (NULL != (li = file_info_getline(file))) {
    char *comma1 =
        find_str(li->line_text, strlen(li->line_text), ",", strlen(","));
    char *comma2 = find_str(comma1 + 1, strlen(comma1 + 1), ",", strlen(","));

    OpenCloseDef_ *def = OpenCloseDefArray_push_back_ref(tokens);
    def->token_name =
        global_intern_range(li->line_text, 0, comma1 - li->line_text);

    char *token_unesc = strip_return_char(comma1, 1, comma2 - comma1);
    const char *open = global_intern(token_unesc);
    def->open.token = open;
    def->open.escaped_token = escape_interned_(open);
    def->open.token_len = strlen(open);
    free(token_unesc);

    token_unesc = strip_return_char(comma2, 1, strlen(comma2 + 1));
    const char *close = global_intern(token_unesc);
    def->close.token = close;
    def->close.escaped_token = escape_interned_(close);
    def->close.token_len = strlen(close);
    free(token_unesc);
    def->open.token_name =
        string_copy_and_append_(li->line_text, comma1 - li->line_text, "_OPEN");
    def->close.token_name = string_copy_and_append_(
        li->line_text, comma1 - li->line_text, "_CLOSE");
  }
}

void lexer_builder_init(LexerBuilder *lb, FileInfo *symbols, FileInfo *keywords,
                        FileInfo *comments, FileInfo *strings) {
  build_token_list_(symbols, &lb->symbols);
  build_token_list_(keywords, &lb->keywords);
  build_open_close_list_(comments, &lb->comments);
  build_open_close_list_(strings, &lb->strings);
  lb->symbols_trie = create_trie_from_symbols_(&lb->symbols);
  lb->keywords_trie = create_trie_from_symbols_(&lb->keywords);
}

void write_source_includes_(LexerBuilder *lb, FILE *file,
                            const char h_file_path[]) {
  // Includes.
  fprintf(file, "#include \"%s\"\n\n", h_file_path);
  fprintf(file, "#include \"language-tools/lexer/lexer_helper.h\"\n");
  fprintf(file, "#include \"file-utils/string_utils.h\"\n");
  fprintf(file, "\n");
}

void write_token_type_enum_(LexerBuilder *lb, FILE *file,
                            const char enum_prefix[]) {
  fprintf(file,
          "typedef enum {\n"
          "  TOKENTYPE_UNKNOWN,\n"
          "  TOKEN_NEWLINE,\n"  // Must stay at index 1 for parser.
          "  TOKEN_WORD,\n"
          "  TOKEN_INTEGER,\n"
          "  TOKEN_FLOATING,\n");
  OpenCloseDefArrayIterator oc_iter;
  OpenCloseDefArray_iterator(&oc_iter, &lb->strings);
  for (; OpenCloseDefArray_has_next(&oc_iter);
       OpenCloseDefArray_next(&oc_iter)) {
    OpenCloseDef_ *open_close_def = OpenCloseDefArray_mutable_value(&oc_iter);
    fprintf(file, "  %s,\n", open_close_def->token_name);
  }
  TokenDefArrayIterator td_iter;
  TokenDefArray_iterator(&td_iter, &lb->symbols);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  TokenDefArray_iterator(&td_iter, &lb->keywords);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "  %s,\n", token_def->token_name);
  }
  fprintf(file, "  TOKEN_NOP\n");
  fprintf(file, "} %sLexType;\n\n", enum_prefix);
}

void write_token_type_to_str_(LexerBuilder *lb, FILE *file,
                              const char fn_prefix[],
                              const char enum_prefix[]) {
  fprintf(file, "const char *%stoken_type_to_str(%sLexType token_type) {\n",
          fn_prefix, enum_prefix);
  fprintf(file, "  switch(token_type) {\n");
  fprintf(file, "    case TOKEN_NEWLINE: return \"\\n\";\n");
  fprintf(file, "    case TOKEN_WORD: return \"TOKEN_WORD\";\n");
  fprintf(file, "    case TOKEN_INTEGER: return \"TOKEN_INTEGER\";\n");
  fprintf(file, "    case TOKEN_FLOATING: return \"TOKEN_FLOATING\";\n");
  OpenCloseDefArrayIterator oc_iter;
  OpenCloseDefArray_iterator(&oc_iter, &lb->strings);
  for (; OpenCloseDefArray_has_next(&oc_iter);
       OpenCloseDefArray_next(&oc_iter)) {
    OpenCloseDef_ *open_close_def = OpenCloseDefArray_mutable_value(&oc_iter);
    fprintf(file, "    case %s: return \"%s\";\n", open_close_def->token_name,
            open_close_def->token_name);
  }
  TokenDefArrayIterator td_iter;
  TokenDefArray_iterator(&td_iter, &lb->symbols);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->escaped_token);
  }
  TokenDefArray_iterator(&td_iter, &lb->keywords);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->escaped_token);
  }
  fprintf(file, "    default: return \"UNKNOWN\";\n  }\n}\n\n");
}

void write_token_name_to_token_type_(LexerBuilder *lb, FILE *file,
                                     const char fn_prefix[],
                                     const char enum_prefix[]) {
  fprintf(file, "%sLexType %stoken_name_to_token_type(const char str[]) {\n",
          enum_prefix, fn_prefix);
  fprintf(file,
          "  if (0 == strcmp(\"TOKEN_NEWLINE\", str)) return TOKEN_NEWLINE;\n");
  fprintf(file, "  if (0 == strcmp(\"TOKEN_WORD\", str)) return TOKEN_WORD;\n");
  fprintf(file,
          "  if (0 == strcmp(\"TOKEN_INTEGER\", str)) return TOKEN_INTEGER;\n");
  fprintf(
      file,
      "  if (0 == strcmp(\"TOKEN_FLOATING\", str)) return TOKEN_FLOATING;\n");
  OpenCloseDefArrayIterator oc_iter;
  OpenCloseDefArray_iterator(&oc_iter, &lb->strings);
  for (; OpenCloseDefArray_has_next(&oc_iter);
       OpenCloseDefArray_next(&oc_iter)) {
    OpenCloseDef_ *open_close_def = OpenCloseDefArray_mutable_value(&oc_iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            open_close_def->token_name, open_close_def->token_name);
  }
  TokenDefArrayIterator td_iter;
  TokenDefArray_iterator(&td_iter, &lb->symbols);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            token_def->token_name, token_def->token_name);
  }
  TokenDefArray_iterator(&td_iter, &lb->keywords);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "  if (0 == strcmp(\"%s\", str)) return %s;\n",
            token_def->token_name, token_def->token_name);
  }
  fprintf(file, "    return 0;\n}\n\n");
}

void write_token_type_to_name_(LexerBuilder *lb, FILE *file,
                               const char fn_prefix[],
                               const char enum_prefix[]) {
  fprintf(file, "const char *%stoken_type_to_name(%sLexType token_type) {\n",
          fn_prefix, enum_prefix);
  fprintf(file, "  switch(token_type) {\n");
  fprintf(file, "    case TOKEN_NEWLINE: return \"TOKEN_NEWLINE\";\n");
  fprintf(file, "    case TOKEN_WORD: return \"TOKEN_WORD\";\n");
  fprintf(file, "    case TOKEN_INTEGER: return \"TOKEN_INTEGER\";\n");
  fprintf(file, "    case TOKEN_FLOATING: return \"TOKEN_FLOATING\";\n");
  OpenCloseDefArrayIterator oc_iter;
  OpenCloseDefArray_iterator(&oc_iter, &lb->strings);
  for (; OpenCloseDefArray_has_next(&oc_iter);
       OpenCloseDefArray_next(&oc_iter)) {
    OpenCloseDef_ *open_close_def = OpenCloseDefArray_mutable_value(&oc_iter);
    fprintf(file, "    case %s: return \"%s\";\n", open_close_def->token_name,
            open_close_def->token_name);
  }
  TokenDefArrayIterator td_iter;
  TokenDefArray_iterator(&td_iter, &lb->symbols);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token_name);
  }
  TokenDefArray_iterator(&td_iter, &lb->keywords);
  for (; TokenDefArray_has_next(&td_iter); TokenDefArray_next(&td_iter)) {
    TokenDef_ *token_def = TokenDefArray_mutable_value(&td_iter);
    fprintf(file, "    case %s: return \"%s\";\n", token_def->token_name,
            token_def->token_name);
  }
  fprintf(file, "    default: return \"UNKNOWN\";\n  }\n}\n\n");
}

void write_switch_for_symbol_resolve_(Trie_ *trie, int index, FILE *file) {
  int i;
  const bool has_children = has_child_trie_(trie);
  if (has_children) {
    fprintf(file, "%*sswitch (word[%d]) {\n", index * 2, "", index - 1);
    for (i = 0; i < TRIE_MAX_CHAR_; ++i) {
      Trie_ *child = trie->chars[i];
      if (NULL == child) {
        continue;
      }
      if (((char)i) == '\\') {
        fprintf(file, "%*scase '\\\\':\n", index * 2, "");
      } else {
        fprintf(file, "%*scase '%c':\n", index * 2, "", (char)i);
      }
      write_switch_for_symbol_resolve_(child, index + 1, file);
    }
    fprintf(file, "%*sdefault: break;\n", index * 2, "");
    fprintf(file, "%*s}\n", index * 2, "");
  }
  if (trie->has) {
    fprintf(file, "%*sreturn %s;\n", index * 2, "", trie->has->token_name);
  }
}

void write_switch_for_keyword_resolve_(Trie_ *trie, int index, FILE *file) {
  int i;
  const bool has_children = has_child_trie_(trie);
  if (trie->has) {
    fprintf(file, "%*sif (word_len == %d) { return %s; }\n", index * 2, "",
            index - 1, trie->has->token_name);
  }
  if (has_children) {
    fprintf(file, "%*sswitch (word[%d]) {\n", index * 2, "", index - 1);
    for (i = 0; i < TRIE_MAX_CHAR_; ++i) {
      Trie_ *child = trie->chars[i];
      if (NULL == child) {
        continue;
      }
      fprintf(file, "%*scase '%c':\n", index * 2, "", (char)i);
      write_switch_for_keyword_resolve_(child, index + 1, file);
    }
    fprintf(file, "%*sdefault: return TOKENTYPE_UNKNOWN;\n", index * 2, "");
    fprintf(file, "%*s}\n", index * 2, "");
  }
  fprintf(file, "%*sreturn TOKENTYPE_UNKNOWN;\n", index * 2, "");
}

void write_resolve_type_(LexerBuilder *lb, FILE *file, const char fn_prefix[],
                         const char enum_prefix[]) {
  fprintf(file, "%sLexType %ssymbol_token_type(const char word[]) {\n",
          enum_prefix, fn_prefix);
  write_switch_for_symbol_resolve_(lb->symbols_trie, 1, file);
  fprintf(file, "  return TOKENTYPE_UNKNOWN;\n");
  fprintf(file, "}\n\n");

  fprintf(file, "%sLexType keyword_type_(const char word[], int word_len) {\n",
          enum_prefix);
  fprintf(file, "  if (word_len <= 0) { return TOKEN_NEWLINE; }\n");
  write_switch_for_keyword_resolve_(lb->keywords_trie, 1, file);
  fprintf(file, "}\n\n");

  fprintf(file, "%sLexType %sresolve_type(const char word[], int word_len) {\n",
          enum_prefix, fn_prefix);
  fprintf(file, "  %sLexType type = %ssymbol_token_type(word);\n", enum_prefix,
          fn_prefix);
  fprintf(file,
          "  if (TOKENTYPE_UNKNOWN == type) { type = keyword_type_(word, "
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

void write_is_start_of_symbol_(LexerBuilder *lb, FILE *file,
                               const char fn_prefix[]) {
  fprintf(file, "bool %sis_start_of_symbol(const char word[]) {\n", fn_prefix);
  int i;
  fprintf(file, "  switch (word[0]) {\n");
  for (i = 0; i < TRIE_MAX_CHAR_; ++i) {
    Trie_ *child = lb->symbols_trie->chars[i];
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

void write_token_type_is_string_(LexerBuilder *lb, FILE *file,
                                 const char fn_prefix[],
                                 const char enum_prefix[]) {
  fprintf(file, "bool %stoken_type_is_string(%sLexType type) {\n", fn_prefix,
          enum_prefix);
  OpenCloseDefArrayIterator iter;
  OpenCloseDefArray_iterator(&iter, &lb->strings);
  fprintf(file, "  switch (type) {\n");
  for (; OpenCloseDefArray_has_next(&iter); OpenCloseDefArray_next(&iter)) {
    OpenCloseDef_ *open_close_def = OpenCloseDefArray_mutable_value(&iter);
    fprintf(file, "    case %s:\n", open_close_def->token_name);
  }
  fprintf(file, "      return true;\n");
  fprintf(file, "    default:\n");
  fprintf(file, "      return false;\n");
  fprintf(file, "  }\n}\n\n");
}

void write_is_start_comment_(LexerBuilder *lb, FILE *file,
                             const char fn_prefix[]) {
  fprintf(file,
          "bool %sis_start_of_comment(const char word[], int "
          "*comment_open_len, char **comment_close) {\n",
          fn_prefix);
  OpenCloseDefArrayIterator iter;
  OpenCloseDefArray_iterator(&iter, &lb->comments);
  for (; OpenCloseDefArray_has_next(&iter); OpenCloseDefArray_next(&iter)) {
    OpenCloseDef_ *def = OpenCloseDefArray_mutable_value(&iter);
    fprintf(file, "  if (0 == strncmp(\"%s\", word, %d)) {\n", def->open.token,
            def->open.token_len);
    fprintf(file, "    *comment_open_len = %d;\n", def->open.token_len);
    fprintf(file, "    *comment_close = \"%s\";\n", def->close.token);
    fprintf(file, "    return true;\n  }\n");
  }
  fprintf(file, "  return false;\n}\n\n");
}

void write_is_start_string_(LexerBuilder *lb, FILE *file,
                            const char fn_prefix[], const char enum_prefix[]) {
  fprintf(
      file,
      "bool %sis_start_of_string(const char word[], %sLexType *string_type, "
      "int *string_open_len, char **string_close) {\n",
      fn_prefix, enum_prefix);
  OpenCloseDefArrayIterator iter;
  OpenCloseDefArray_iterator(&iter, &lb->strings);
  for (; OpenCloseDefArray_has_next(&iter); OpenCloseDefArray_next(&iter)) {
    OpenCloseDef_ *def = OpenCloseDefArray_mutable_value(&iter);
    fprintf(file, "  if (0 == strncmp(\"%s\", word, %d)) {\n",
            def->open.escaped_token, def->open.token_len);
    fprintf(file, "    *string_type = %s;\n", def->token_name);
    fprintf(file, "    *string_open_len = %d;\n", def->open.token_len);
    fprintf(file, "    *string_close = \"%s\";\n", def->close.escaped_token);
    fprintf(file, "    return true;\n  }\n");
  }
  fprintf(file, "  return false;\n}\n\n");
}

const char TOKENIZE_FUNCTIONS_TEXT_[] =
    "\n\
int tokenize_number_(const LineInfo *li, TokenArray *tokens, int col_num) {\n\
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
  *TokenArray_push_back_ref(tokens) = token;\n\
  return col_num;\n\
}\n\
\n\
int tokenize_symbol_(const LineInfo *li, TokenArray *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  %sLexType type = %ssymbol_token_type(line + col_num);\n\
  if (TOKENTYPE_UNKNOWN == type) {\n\
    fprintf(stderr, \"UNKNOWN TOKEN\\n\");\n\
    exit(1);\n\
  }\n\
  const int token_length = strlen(%stoken_type_to_str(type));\n\
  Token *token =\n\
      token_create(type, li->line_num, col_num, line + col_num, token_length);\n\
  *TokenArray_push_back_ref(tokens) = token;\n\
  col_num += token_length;\n\
  return col_num;\n\
}\n\
\n\
int tokenize_word_(const LineInfo *li, TokenArray *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  int start = col_num++;\n\
  while (is_alphanumeric(line[col_num])) {\n\
    ++col_num;\n\
  }\n\
  %sLexType token_type = keyword_type_(line + start, col_num - start);\n\
  Token *token = token_create(\n\
      token_type == TOKENTYPE_UNKNOWN ? TOKEN_WORD : token_type,\n\
      li->line_num,\n\
      start,\n\
      line + start,\n\
      col_num - start);\n\
  *TokenArray_push_back_ref(tokens) = token;\n\
  return col_num;\n\
}\n\
\n\
int tokenize_newline_(const LineInfo *li, TokenArray *tokens, int col_num) {\n\
  char *line = li->line_text;\n\
  Token *last = TokenArray_is_empty(tokens) ? NULL : TokenArray_get_unchecked(tokens, TokenArray_size(tokens) - 1);\n\
  if (NULL == last || last->type != TOKEN_NEWLINE) {\n\
    Token *token =\n\
        token_create(TOKEN_NEWLINE, li->line_num, col_num, line + col_num, 1);\n\
    *TokenArray_push_back_ref(tokens) = token;\n\
  }\n\
  ++col_num;\n\
  return col_num;\n\
}\n\
\n\
bool lexer_tokenize_line_(FileInfo *fi, TokenArray *tokens, bool *in_comment, bool *in_string,\n\
                          char **comment_end, char **string_end, %sLexType *string_type, char **string_buffer) {\n\
  LineInfo *li = file_info_getline(fi);\n\
  if (NULL == li) {\n\
    return false;\n\
  }\n\
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
      char *eoc = (strlen(*comment_end) > strlen(line) - col_num) ? NULL : find_str(line + col_num,\n\
                            strlen(line) - col_num, *comment_end,\n\
                            strlen(*comment_end));\n\
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
        *string_buffer = malloc(sizeof(char) * (col_num - string_start_col + 1));\n\
        memcpy(*string_buffer, line + string_start_col, col_num - string_start_col);\n\
        (*string_buffer)[col_num - string_start_col] = '\\0';\n\
      } else {\n\
        char *new_string_buffer =\n\
            malloc(sizeof(char) *( strlen(*string_buffer) + col_num - string_start_col + 1));\n\
        memcpy(new_string_buffer, *string_buffer, strlen(*string_buffer));\n\
        memcpy(new_string_buffer + strlen(*string_buffer),  line + string_start_col, col_num - string_start_col);\n\
        new_string_buffer[strlen(*string_buffer) + col_num - string_start_col] = '\\0';\n\
        free(*string_buffer);\n\
        *string_buffer = new_string_buffer;\n\
      }\n\
      // End of string not found.\n\
      if (line + strlen(line) == eos) {\n\
        return true;\n\
      }\n\
      Token *token = token_create(*string_type,\n\
                   li->line_num, string_start_col, *string_buffer, strlen(*string_buffer));\n\
      free(*string_buffer);\n\
      *string_buffer = NULL;\n\
      *TokenArray_push_back_ref(tokens) = token;\n\
      col_num = eos - line + strlen(*string_end);\n\
      *in_string = false;\n\
      *string_end = NULL;\n\
      continue;\n\
    }\n\
    int comment_open_len;\n\
    if (%sis_start_of_comment(line + col_num, &comment_open_len, comment_end)) {\n\
      *in_comment = true;\n\
      col_num += comment_open_len;\n\
      continue;\n\
    }\n\
    int string_open_len;\n\
    if (%sis_start_of_string(line + col_num, string_type, &string_open_len, string_end)) {\n\
      *in_string = true;\n\
      col_num += string_open_len;\n\
      string_start_col = col_num;\n\
      continue;\n\
    }\n\
    if ('\\0' == line[col_num]) {\n\
      continue;\n\
    } else if (is_numeric(line[col_num])) {\n\
      col_num = tokenize_number_(li, tokens, col_num);\n\
    } else if (%sis_start_of_symbol(line + col_num)) {\n\
      col_num = tokenize_symbol_(li, tokens, col_num);\n\
    } else if (is_alphanumeric(line[col_num])) {\n\
      col_num = tokenize_word_(li, tokens, col_num);\n\
    } else if ('\\n' == line[col_num] || '\\r' == line[col_num]) {\n\
      col_num = tokenize_newline_(li, tokens, col_num);\n\
    } else {\n\
      printf(\"%%d:%%d \\\"%%c\\\"\\n\", li->line_num, col_num, line[col_num]);\n\
      printf(\"line: \\\"%%s\\\"\\n\", line);\n\
      fflush(stdout);\n\
      fprintf(stderr, \"NEVER HERE!\\n\");\n\
      exit(1);\n\
    }\n\
  }\n\
  return true;\n\
}\n\
\n\
void %slexer_tokenize_line(FileInfo *file, TokenArray *tokens) {\n\
  bool in_comment = false;\n\
  char *comment_end = NULL;\n\
  bool in_string = false;\n\
  char *string_end = NULL;\n\
  %sLexType string_type = TOKENTYPE_UNKNOWN;\n\
  char *string_buffer = NULL;\n\
  lexer_tokenize_line_(file, tokens, &in_comment, &in_string, &comment_end, &string_end, &string_type, &string_buffer);\n\
}\n\
void %slexer_tokenize(FileInfo *file, TokenArray *tokens) {\n\
  bool in_comment = false;\n\
  char *comment_end = NULL;\n\
  bool in_string = false;\n\
  char *string_end = NULL;\n\
  %sLexType string_type;\n\
  char *string_buffer = NULL;\n\
  while (lexer_tokenize_line_(file, tokens, &in_comment, &in_string, &comment_end, &string_end, &string_type, &string_buffer))\n\
    ;\n\
}\n";

void lexer_builder_write_c_file(LexerBuilder *lb, FILE *file,
                                const char h_file_path[],
                                const char fn_prefix[],
                                const char enum_prefix[]) {
  write_source_includes_(lb, file, h_file_path);
  write_token_type_to_str_(lb, file, fn_prefix, enum_prefix);
  write_token_type_to_name_(lb, file, fn_prefix, enum_prefix);
  write_token_name_to_token_type_(lb, file, fn_prefix, enum_prefix);
  write_resolve_type_(lb, file, fn_prefix, enum_prefix);
  write_is_start_of_symbol_(lb, file, fn_prefix);
  write_is_start_comment_(lb, file, fn_prefix);
  write_is_start_string_(lb, file, fn_prefix, enum_prefix);
  write_token_type_is_string_(lb, file, fn_prefix, enum_prefix);
  fprintf(file, TOKENIZE_FUNCTIONS_TEXT_, enum_prefix, fn_prefix, fn_prefix,
          enum_prefix, enum_prefix, fn_prefix, fn_prefix, fn_prefix, fn_prefix,
          enum_prefix, fn_prefix, enum_prefix);
}

void lexer_builder_write_h_file(LexerBuilder *lb, FILE *file,
                                const char fn_prefix[],
                                const char enum_prefix[]) {
  fprintf(file, "#ifndef COM_GITHUB_LANGUAGE_TOOLS_LEXER_CUSTOM_LEXER_H_%s\n",
          fn_prefix);
  fprintf(
      file, "#define COM_GITHUB_LANGUAGE_TOOLS_LEXER_CUSTOM_LEXER_H_%s\n\n",

#ifdef __cplusplus
      extern "C" {
#endif
          fn_prefix);
          fprintf(file, "#include <stdbool.h>\n\n");
          fprintf(file, "#include \"file-utils/file_info.h\"\n");
          fprintf(file, "#include \"language-tools/lexer/token.h\"\n\n");
          write_token_type_enum_(lb, file, enum_prefix);
          fprintf(file, "%sLexType %ssymbol_token_type(const char word[]);\n",
                  enum_prefix, fn_prefix);
          fprintf(file,
                  "const char *%stoken_type_to_str(%sLexType token_type);\n",
                  fn_prefix, enum_prefix);
          fprintf(file,
                  "const char *%stoken_type_to_name(%sLexType token_type);\n",
                  fn_prefix, enum_prefix);
          fprintf(file,
                  "%sLexType %stoken_name_to_token_type(const char str[]);\n",
                  enum_prefix, fn_prefix);
          fprintf(
              file,
              "%sLexType %sresolve_type(const char word[], int word_len);\n",
              enum_prefix, fn_prefix);
          fprintf(file, "bool %sis_start_of_symbol(const char word[]);\n",
                  fn_prefix);
          fprintf(file,
                  "bool %sis_start_of_comment(const char word[], int "
                  "*comment_open_len, "
                  "char **comment_close);\n",
                  fn_prefix);
          fprintf(file,
                  "bool %sis_start_of_string(const char word[], %sLexType "
                  "*string_type, "
                  "int *string_open_len, char **string_close);\n",
                  fn_prefix, enum_prefix);
          fprintf(file, "bool %stoken_type_is_string(%sLexType type);\n",
                  fn_prefix, enum_prefix);
          fprintf(file,
                  "void %slexer_tokenize_line(FileInfo *file, TokenArray "
                  "*tokens);\n",
                  fn_prefix);
          fprintf(
              file,
              "void %slexer_tokenize(FileInfo *file, TokenArray *tokens);\n",
              fn_prefix);
          fprintf(file,
                  "\n#ifdef __cplusplus
      }
#endif

#endif /* "
                  "COM_GITHUB_LANGUAGE_TOOLS_LEXER_CUSTOM_LEXER_H_%s */\n",
      fn_prefix);
}

void lexer_builder_finalize(LexerBuilder *lb) {
  TokenDefArray_finalize(&lb->symbols);
  TokenDefArray_finalize(&lb->keywords);
  trie_delete_(lb->symbols_trie);
  trie_delete_(lb->keywords_trie);
  OpenCloseDefArray_finalize(&lb->comments);
}
