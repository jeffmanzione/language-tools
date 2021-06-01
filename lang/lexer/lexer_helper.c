#include "lang/lexer/lexer_helper.h"

#include <stdbool.h>

inline bool is_numeric(const char c) { return ('0' <= c && '9' >= c); }

inline bool is_number(const char c) { return is_numeric(c) || '.' == c; }

inline bool is_alphabetic(const char c) {
  return ('A' <= c && 'Z' >= c) || ('a' <= c && 'z' >= c);
}

inline bool is_alphanumeric(const char c) {
  return is_numeric(c) || is_alphabetic(c) || '_' == c || '$' == c;
}

bool is_any_space(const char c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    return true;
  default:
    return false;
  }
}

inline bool is_whitespace(const char c) {
  return ' ' == c || '\t' == c || '\r' == c;
}

char char_unesc(char u) {
  switch (u) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case '\\':
    return '\\';
  case '\'':
    return '\'';
  case '\"':
    return '\"';
  case '\?':
    return '\?';
  default:
    return u;
  }
}