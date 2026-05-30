#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_HELPER_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_HELPER_H_

#include <stdbool.h>

bool is_number(const char c);
bool is_numeric(const char c);
bool is_alphabetic(const char c);
bool is_alphanumeric(const char c);
bool is_whitespace(const char c);
bool is_any_space(const char c);
char char_unesc(char u);
char *escape_string(const char str[]);
char *strip_return_char(const char *str, int start, int end);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_LEXER_LEXER_HELPER_H_ */