#include "language-tools/lexer/lexer_helper.h"

#include <stdlib.h>

bool is_numeric(const char c) { return ('0' <= c && '9' >= c); }

bool is_number(const char c) { return is_numeric(c) || '.' == c; }

bool is_alphabetic(const char c) {
  return ('A' <= c && 'Z' >= c) || ('a' <= c && 'z' >= c);
}

bool is_alphanumeric(const char c) {
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

bool is_whitespace(const char c) { return ' ' == c || '\t' == c || '\r' == c; }

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

#define DEFAULT_ESCAPED_STRING_SZ 32

static bool should_escape_(char c) {
  switch (c) {
    case '\'':
    case '\"':
    case '\n':
    case '\r':
    case '\\':
      return true;
    default:
      return false;
  }
}

static char excape_char_(char c) {
  switch (c) {
    case '\n':
      return 'n';
    case '\t':
      return 't';
    case '\r':
      return 'r';
    case '\\':
      return '\\';
    default:
      return c;
  }
}

char *escape_string(const char str[]) {
  if (NULL == str) {
    return NULL;
  }
  const char *ptr = str;
  char c;
  char *escaped_str = malloc(sizeof(char) * DEFAULT_ESCAPED_STRING_SZ);
  int escaped_len = 0, escaped_buffer_sz = DEFAULT_ESCAPED_STRING_SZ;
  while ('\0' != (c = *ptr)) {
    if (escaped_len > escaped_buffer_sz - 3) {
      escaped_str = realloc(
          escaped_str,
          sizeof(char) * (escaped_buffer_sz += DEFAULT_ESCAPED_STRING_SZ));
    }
    if ('\r' == c) {
      ptr++;
      continue;
    }
    if (should_escape_(c)) {
      escaped_str[escaped_len++] = '\\';
    }
    escaped_str[escaped_len++] = excape_char_(c);
    ptr++;
  }
  escaped_str[escaped_len] = '\0';
  return realloc(escaped_str, sizeof(char) * (escaped_len + 1));
}

char *strip_return_char(const char *str, int start, int end) {
  if (NULL == str) {
    return NULL;
  }
  const char *ptr = str;
  char c;
  char *new_str = malloc(sizeof(char) * DEFAULT_ESCAPED_STRING_SZ);
  int len = 0, buffer_sz = DEFAULT_ESCAPED_STRING_SZ;
  for (int i = start; i < end; ++i) {
    c = ptr[i];
    if (len > buffer_sz - 3) {
      new_str = realloc(
          new_str, sizeof(char) * (buffer_sz += DEFAULT_ESCAPED_STRING_SZ));
    }
    if ('\r' == c) {
      i++;
      continue;
    }
    new_str[len++] = excape_char_(c);
  }
  new_str[len] = '\0';
  return realloc(new_str, sizeof(char) * (len + 1));
}