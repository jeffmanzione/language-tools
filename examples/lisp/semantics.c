#include "examples/lisp/semantics.h"

#include "examples/lisp/lisp_lexer.h"

POPULATE_IMPL(expression_function, const SyntaxTree *stree,
              SemanticAnalyzer *analyzer) {
  ExpressionTreeArray_init(&expression_function->args);
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
      fprintf(stderr, "Unknown function type.\n");
      exit(1);
  }

  const SyntaxTree *args = CHILD_SYNTAX_AT(stree, 2);
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
  ExpressionTreeArrayIterator it;
  ExpressionTreeArray_iterator(&it, &expression_function->args);
  for (; ExpressionTreeArray_has_next(&it); ExpressionTreeArray_next(&it)) {
    ExpressionTree *child = *ExpressionTreeArray_mutable_value(&it);
    semantic_analyzer_delete(analyzer, child);
  }
}

POPULATE_IMPL(expression, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  expression->floating = atof(stree->token->text);
}

DELETE_IMPL(expression, SemanticAnalyzer *analyzer) {}

void init_semantics(SAMap *populators, SAMap *producers, SAMap *deleters) {
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
    ExpressionTreeArrayIterator it;
    ExpressionTreeArray_iterator(&it, &f->args);
    switch (f->func) {
      case FUNC_AND:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value = value && evaluate_lisp_expression(
                               *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_OR:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value = value || evaluate_lisp_expression(
                               *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_NOT:
        value = !evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        break;
      case FUNC_IF:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        if (value) {
          value = evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        } else {
          // Skip the true condition.
          ExpressionTreeArray_next(&it);
          value = evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_ADD:
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value += evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_SUBTRACT:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value -= evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_MULTIPLY:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value *= evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      case FUNC_DIVIDE:
        value = evaluate_lisp_expression(
            *ExpressionTreeArray_mutable_value(&it), file);
        ExpressionTreeArray_next(&it);
        for (; ExpressionTreeArray_has_next(&it);
             ExpressionTreeArray_next(&it)) {
          value /= evaluate_lisp_expression(
              *ExpressionTreeArray_mutable_value(&it), file);
        }
        break;
      default:
        fprintf(stderr, "Unknown function\n");
        exit(1);
    }
    return value;
  } else {
    fprintf(stderr, "Unknown expression.");
    exit(1);
  }
  return 0;
}