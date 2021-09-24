#include "c:\users\jeffr\git\language-tools\test.h"

#include "lang/lexer/token.h"
#include "lang/parser/production_lexer/production_lexer.h"

SyntaxTree *rule_epsilon(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || KEYWORD_EPSILON != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_token__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || KEYWORD_TOKEN != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_token__token1(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_COLON != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_token__token2(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || TOKEN_WORD != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

SyntaxTree *rule_token(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_token__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_token__token1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_token__token2(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

inline SyntaxTree *rule_rule__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || KEYWORD_RULE != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_rule__token1(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_COLON != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_rule__token2(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || TOKEN_WORD != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

SyntaxTree *rule_rule(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_rule__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_rule__token1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_rule__token2(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

inline SyntaxTree *rule_list1__and0__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_COMMA != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_list1__and0(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_list1__and0__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_production_expression(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_list1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

SyntaxTree *rule_list1(Parser *parser) {
  {
    SyntaxTree *st_child = rule_list1__and0(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  {
    SyntaxTree *st_child = &MATCH_EPSILON;
    if (st_child->matched) {
      return st_child;
    }
  }
  return &NO_MATCH;
}

SyntaxTree *rule_list(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_production_expression(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_list1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

inline SyntaxTree *rule_and__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || KEYWORD_AND != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_and__token1(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_LPAREN != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_and__token3(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_RPAREN != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

SyntaxTree *rule_and(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_and__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_and__token1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_list(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_and__token3(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

inline SyntaxTree *rule_or__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || KEYWORD_OR != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_or__token1(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_LPAREN != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_or__token3(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_RPAREN != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

SyntaxTree *rule_or(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_or__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_or__token1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_list(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_or__token3(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

SyntaxTree *rule_production_expression(Parser *parser) {
  {
    SyntaxTree *st_child = rule_and(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  {
    SyntaxTree *st_child = rule_or(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  {
    SyntaxTree *st_child = rule_rule(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  {
    SyntaxTree *st_child = rule_token(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  {
    SyntaxTree *st_child = rule_epsilon(parser);
    if (st_child->matched) {
      return st_child;
    }
  }
  return &NO_MATCH;
}

inline SyntaxTree *rule_production_rule__token0(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || TOKEN_WORD != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_production_rule__token1(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMBOL_ARROW != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

inline SyntaxTree *rule_production_rule__token3(Parser *parser) {
  Token *token = parser_next(parser);
  if (NULL == token || SYMOBL_SEMICOLON != token->type) {
    return &NO_MATCH;
  }
  return match(parser);
}

SyntaxTree *rule_production_rule(Parser *parser) {
  SyntaxTree *st = parser_create_st(parser);
  {
    SyntaxTree *st_child = rule_production_rule__token0(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_production_rule__token1(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_production_expression(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  {
    SyntaxTree *st_child = rule_production_rule__token3(parser);
    if (!st->matched) {
      parser_delete_st(parser, st);
      return &NO_MATCH;
    }
    syntax_tree_add_child(st, st_child);
  }
  return st;
}

