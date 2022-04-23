#include "alloc/alloc.h"
#include "alloc/arena/intern.h"
#include "debug/debug.h"
#include "examples/lisp/lisp_lexer.h"
#include "examples/lisp/lisp_parser.h"
#include "examples/lisp/semantics.h"
#include "lang/lexer/token.h"
#include "struct/q.h"
#include "util/file/file_info.h"
#include "util/file/sfile.h"

int main(int argc, const char *args[]) {
  alloc_init();
  intern_init();

  FileInfo *file = file_info_file(stdin);

  while (true) {
    Q tokens;
    Q_init(&tokens);

    printf("> ");

    lexer_tokenize_line(file, &tokens);

    Parser parser;
    parser_init(&parser, rule_expression,
                /*ignore_newline=*/false);
    SyntaxTree *stree = parser_parse(&parser, &tokens);
    stree = parser_prune_newlines(&parser, stree);
    // syntax_tree_print(stree, 0, stdout);
    // printf("\n");

    SemanticAnalyzer analyzer;
    semantic_analyzer_init(&analyzer, init_semantics);
    ExpressionTree *etree = semantic_analyzer_populate(&analyzer, stree);

    double result = evaluate_lisp_expression(etree, stdout);
    printf("<-- %0.4f\n", result);

    semantic_analyzer_delete(&analyzer, etree);
    semantic_analyzer_finalize(&analyzer);

    parser_delete_st(&parser, stree);
    parser_finalize(&parser);
    Q_finalize(&tokens);
  }

  file_info_delete(file);

  token_finalize_all();
  intern_finalize();
  alloc_finalize();
}