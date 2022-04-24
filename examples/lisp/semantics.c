#include "examples/lisp/semantics.h"

#include "debug/debug.h"
#include "examples/lisp/lisp_lexer.h"

POPULATE_IMPL(expression_function, const SyntaxTree *stree,
              SemanticAnalyzer *analyzer) {
  alist_init(&expression_function->args, ExpressionTree *, DEFAULT_ARRAY_SZ);
  switch (CHILD_SYNTAX_AT(stree, 1)->token->type) {
  case KEYWORD_AND:
    expression_function->func = FUNC_AND;
    break;
  case KEYWORD_OR:
    expression_function->func = FUNC_OR;
    break;
  case KEYWORD_NOT:
    expression_function->func = FUNC_NOT;
    break;
  case KEYWORD_IF:
    expression_function->func = FUNC_IF;
    break;
  case SYMBOL_PLUS:
    expression_function->func = FUNC_ADD;
    break;
  case SYMBOL_MINUS:
    expression_function->func = FUNC_SUBTRACT;
    break;
  case SYMBOL_STAR:
    expression_function->func = FUNC_MULTIPLY;
    break;
  case SYMBOL_FSLASH:
    expression_function->func = FUNC_DIVIDE;
    break;
  default:
    FATALF("Unknown function type.");
  }

  SyntaxTree *args = CHILD_SYNTAX_AT(stree, 2);
  while (true) {
    if (IS_SYNTAX(args, rule_expression_function_items) ||
        IS_SYNTAX(args, rule_expression_function_items1)) {
      APPEND_TREE(analyzer, &expression_function->args,
                  CHILD_SYNTAX_AT(args, 0));
      args = CHILD_SYNTAX_AT(args, 1);
    } else {
      APPEND_TREE(analyzer, &expression_function->args, args);
      break;
    }
  }
}

DELETE_IMPL(expression_function, SemanticAnalyzer *analyzer) {
  for (AL_iter iter = alist_iter(&expression_function->args); al_has(&iter);
       al_inc(&iter)) {
    ExpressionTree *child = *(ExpressionTree **)al_value(&iter);
    semantic_analyzer_delete(analyzer, child);
  }
}

POPULATE_IMPL(expression, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  expression->floating = atof(stree->token->text);
}

DELETE_IMPL(expression, SemanticAnalyzer *analyzer) {}

void init_semantics(Map *populators, Map *producers, Map *deleters) {
  REGISTER_EXPRESSION(expression_function);
  REGISTER_EXPRESSION(expression);
}

double evaluate_lisp_expression(ExpressionTree *tree, FILE *file) {
  if (IS_EXPRESSION(tree, expression)) {
    return EXTRACT_EXPRESSION(tree, expression)->floating;
  } else if (IS_EXPRESSION(tree, expression_function)) {
    Expression_expression_function *f =
        EXTRACT_EXPRESSION(tree, expression_function);
    double value = 0;
    AL_iter iter = alist_iter(&f->args);
    switch (f->func) {
    case FUNC_AND:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      for (; al_has(&iter); al_inc(&iter)) {
        value = value && evaluate_lisp_expression(
                             *(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_OR:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      for (; al_has(&iter); al_inc(&iter)) {
        value = value || evaluate_lisp_expression(
                             *(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_NOT:
      value =
          !evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      break;
    case FUNC_IF:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      if (value) {
        value =
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      } else {
        // Skip the true condition.
        al_inc(&iter);
        value =
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_ADD:
      for (; al_has(&iter); al_inc(&iter)) {
        value +=
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_SUBTRACT:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      for (; al_has(&iter); al_inc(&iter)) {
        value -=
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_MULTIPLY:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      for (; al_has(&iter); al_inc(&iter)) {
        value *=
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      }
      break;
    case FUNC_DIVIDE:
      value =
          evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      al_inc(&iter);
      for (; al_has(&iter); al_inc(&iter)) {
        value /=
            evaluate_lisp_expression(*(ExpressionTree **)al_value(&iter), file);
      }
      break;
    default:
      FATALF("Unknown function");
    }
    return value;
  } else {
    FATALF("Unknown expression.");
  }
  return 0;
}