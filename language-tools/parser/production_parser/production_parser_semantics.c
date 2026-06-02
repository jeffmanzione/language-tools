#include "language-tools/parser/production_parser/production_parser_semantics.h"

POPULATE_IMPL(epsilon, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {}

DELETE_IMPL(epsilon, SemanticAnalyzer *analyzer) {}

POPULATE_IMPL(token, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  const SyntaxTree *tok = CHILD_SYNTAX_AT(stree, 2);
  if (!HAS_TOKEN(tok)) {
    fprintf(stderr, "Rule token must have a token.\n");
    exit(1);
  }
  token->token_type = tok->token->text;
}

DELETE_IMPL(token, SemanticAnalyzer *analyzer) {}

POPULATE_IMPL(rule, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  const SyntaxTree *rule_name = CHILD_SYNTAX_AT(stree, 2);
  if (!HAS_TOKEN(rule_name)) {
    fprintf(stderr, "Rule rule must have a rule_name.\n");
    exit(1);
  }
  rule->rule_name = rule_name->token->text;
}

DELETE_IMPL(rule, SemanticAnalyzer *analyzer) {}

void populate_list1_(SemanticAnalyzer *analyzer, const SyntaxTree *list1,
                     ExpressionTreeArray *expressions) {
  EXPECT_TYPE(list1, rule_list1);
  const SyntaxTree *first = CHILD_SYNTAX_AT(list1, 1);
  ExpressionTree *first_rule = semantic_analyzer_populate(analyzer, first);
  ExpressionTreeArray_push_back(expressions, first_rule);

  const SyntaxTree *tail = CHILD_SYNTAX_AT(list1, 2);
  if (NULL != tail) {
    populate_list1_(analyzer, tail, expressions);
  }
}

void populate_list_(SemanticAnalyzer *analyzer, const SyntaxTree *child_list,
                    ExpressionTreeArray *expressions) {
  EXPECT_TYPE(child_list, rule_list);
  ExpressionTreeArray_init(expressions);

  const SyntaxTree *first = CHILD_SYNTAX_AT(child_list, 0);
  ExpressionTree *first_rule = semantic_analyzer_populate(analyzer, first);
  ExpressionTreeArray_push_back(expressions, first_rule);

  const SyntaxTree *tail = CHILD_SYNTAX_AT(child_list, 1);
  populate_list1_(analyzer, tail, expressions);
}

POPULATE_IMPL(and, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  const SyntaxTree *child_list = CHILD_SYNTAX_AT(stree, 2);
  populate_list_(analyzer, child_list, &and->expressions);
}

DELETE_IMPL(and, SemanticAnalyzer *analyzer) {
  ExpressionTreeArrayIterator expressions;
  ExpressionTreeArray_iterator(&expressions, &and->expressions);
  for (; ExpressionTreeArray_has_next(&expressions);
       ExpressionTreeArray_next(&expressions)) {
    ExpressionTree *etree = *ExpressionTreeArray_mutable_value(&expressions);
    semantic_analyzer_delete(analyzer, etree);
  }
  ExpressionTreeArray_finalize(&and->expressions);
}

POPULATE_IMPL(or, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  const SyntaxTree *child_list = CHILD_SYNTAX_AT(stree, 2);
  populate_list_(analyzer, child_list, & or->expressions);
}

DELETE_IMPL(or, SemanticAnalyzer *analyzer) {
  ExpressionTreeArrayIterator expressions;
  ExpressionTreeArray_iterator(&expressions, & or->expressions);
  for (; ExpressionTreeArray_has_next(&expressions);
       ExpressionTreeArray_next(&expressions)) {
    ExpressionTree *etree = *ExpressionTreeArray_mutable_value(&expressions);
    semantic_analyzer_delete(analyzer, etree);
  }
  ExpressionTreeArray_finalize(& or->expressions);
}

POPULATE_IMPL(sequence, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  if (SyntaxTreeArray_size(&stree->children) != 4 &&
      SyntaxTreeArray_size(&stree->children) != 6) {
    fprintf(stderr, "LIST can only have 1 or 2 entries.\n");
    exit(1);
  }
  if (SyntaxTreeArray_size(&stree->children) == 6) {
    sequence->delim =
        semantic_analyzer_populate(analyzer, CHILD_SYNTAX_AT(stree, 2));
    sequence->item =
        semantic_analyzer_populate(analyzer, CHILD_SYNTAX_AT(stree, 4));
  } else {
    sequence->delim = NULL;
    sequence->item =
        semantic_analyzer_populate(analyzer, CHILD_SYNTAX_AT(stree, 2));
  }
}

DELETE_IMPL(sequence, SemanticAnalyzer *analyzer) {
  if (NULL != sequence->delim) {
    semantic_analyzer_delete(analyzer, sequence->delim);
  }
  semantic_analyzer_delete(analyzer, sequence->item);
}

POPULATE_IMPL(optional, const SyntaxTree *stree, SemanticAnalyzer *analyzer) {
  const SyntaxTree *exp = CHILD_SYNTAX_AT(stree, 2);
  optional->expression = semantic_analyzer_populate(analyzer, exp);
}

DELETE_IMPL(optional, SemanticAnalyzer *analyzer) {
  semantic_analyzer_delete(analyzer, optional->expression);
}

POPULATE_IMPL(production_rule, const SyntaxTree *stree,
              SemanticAnalyzer *analyzer) {
  if (SyntaxTreeArray_size(&stree->children) < 3) {
    fprintf(stderr, "Rule production_rule must have 3 children, was %ld\n",
            SyntaxTreeArray_size(&stree->children));
    exit(1);
  }
  const SyntaxTree *identifier = CHILD_SYNTAX_AT(stree, 0);
  const SyntaxTree *expression = CHILD_SYNTAX_AT(stree, 2);
  if (!HAS_TOKEN(identifier)) {
    fprintf(stderr, "First child of production_rule must be an identifier.\n");
    exit(1);
  }
  production_rule->rule_name = TOKEN_TEXT_FOR(identifier);
  // printf("%s\n", production_rule->rule_name);
  // syntax_tree_print(expression, 0, stdout);
  // printf("\n\n");

  production_rule->expression =
      semantic_analyzer_populate(analyzer, expression);
}

DELETE_IMPL(production_rule, SemanticAnalyzer *analyzer) {
  if (NULL != production_rule->expression) {
    semantic_analyzer_delete(analyzer, production_rule->expression);
  }
}

void _populate_production_rule_set1(SemanticAnalyzer *analyzer,
                                    const SyntaxTree *stree,
                                    ExpressionTreeArray *rules) {
  EXPECT_TYPE(stree, rule_production_rule_set1);
  SyntaxTreeArrayIterator children;
  SyntaxTreeArray_iterator(&children, &stree->children);
  for (; SyntaxTreeArray_has_next(&children); SyntaxTreeArray_next(&children)) {
    const SyntaxTree *st_child = *SyntaxTreeArray_value(&children);
    if (IS_SYNTAX(st_child, rule_production_rule)) {
      ExpressionTree *exp = semantic_analyzer_populate(analyzer, st_child);
      ExpressionTreeArray_push_back(rules, exp);
    } else if (IS_SYNTAX(st_child, rule_production_rule_set1)) {
      _populate_production_rule_set1(analyzer, st_child, rules);
    }
  }
}

POPULATE_IMPL(production_rule_set, const SyntaxTree *stree,
              SemanticAnalyzer *analyzer) {
  ExpressionTreeArray_init(&production_rule_set->rules);
  const SyntaxTree *first = CHILD_SYNTAX_AT(stree, 0);
  ExpressionTree *first_rule = semantic_analyzer_populate(analyzer, first);
  ExpressionTreeArray_push_back(&production_rule_set->rules, first_rule);
  if (SyntaxTreeArray_size(&stree->children) > 1) {
    _populate_production_rule_set1(analyzer, CHILD_SYNTAX_AT(stree, 1),
                                   &production_rule_set->rules);
  }
}

DELETE_IMPL(production_rule_set, SemanticAnalyzer *analyzer) {
  ExpressionTreeArrayIterator iter;
  ExpressionTreeArray_iterator(&iter, &production_rule_set->rules);
  for (; ExpressionTreeArray_has_next(&iter); ExpressionTreeArray_next(&iter)) {
    semantic_analyzer_delete(analyzer,
                             *ExpressionTreeArray_mutable_value(&iter));
  }
  ExpressionTreeArray_finalize(&production_rule_set->rules);
}

void production_parser_init_semantics(SAMap *populators, SAMap *producers,
                                      SAMap *deleters) {
  REGISTER_EXPRESSION(epsilon);
  REGISTER_EXPRESSION(token);
  REGISTER_EXPRESSION(rule);
  REGISTER_EXPRESSION(and);
  REGISTER_EXPRESSION(or);
  REGISTER_EXPRESSION(optional);
  REGISTER_EXPRESSION(sequence);
  REGISTER_EXPRESSION(production_rule);
  REGISTER_EXPRESSION(production_rule_set);
}