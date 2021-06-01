#ifndef LANGUAGE_TOOLS_LANG_LEXER_LEXER_HELPER_H_
#define LANGUAGE_TOOLS_LANG_LEXER_LEXER_HELPER_H_

bool is_number(const char c);
bool is_numeric(const char c);
bool is_alphabetic(const char c);
bool is_alphanumeric(const char c);
bool is_whitespace(const char c);
bool is_any_space(const char c);
char char_unesc(char u);

// bool is_special_char(const char c);

#endif /* LANGUAGE_TOOLS_LANG_LEXER_LEXER_HELPER_H_ */