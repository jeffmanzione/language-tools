#ifndef COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_EXAMPLES_LISP_SEMANTICS_H_
#define COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_EXAMPLES_LISP_SEMANTICS_H_

#include "examples/lisp/lisp_parser.h"
#include "language-tools/lexer/token.h"
#include "language-tools/semantic_analyzer/expression_tree.h"
#include "language-tools/semantic_analyzer/semantic_analyzer.h"
#include "struct/alist.h"

DEFINE_EXPRESSION(expression_function) {
  enum {
    FUNC_AND,
    FUNC_OR,
    FUNC_NOT,
    FUNC_IF,
    FUNC_ADD,
    FUNC_SUBTRACT,
    FUNC_MULTIPLY,
    FUNC_DIVIDE,
  } func;
  AList args; /* ExpressionTree* */
};

DEFINE_EXPRESSION(expression) { double floating; };

void init_semantics(Map *populators, Map *producers, Map *deleters);

double evaluate_lisp_expression(ExpressionTree *tree, FILE *file);

#endif /* COM_GITHUB_JEFFMANZIONE_LANGUAGE_TOOLS_EXAMPLES_LISP_SEMANTICS_H_*/