#include "examples/lisp/lisp_lexer.h"
#include "examples/lisp/lisp_parser.h"
#include "examples/lisp/semantics.h"
#include "file-utils/file_info.h"
#include "file-utils/sfile.h"
#include "language-tools/intern.h"
#include "language-tools/lexer/token.h"

int main(int argc, const char *args[]) {
  global_string_intern_pool_init();

  FileInfo *file = file_info_file(stdin);

  while (true) {
    TokenArray tokens;
    TokenArray_init(&tokens);

    printf("> ");

    lisp_lexer_tokenize_line(file, &tokens);

    Parser parser;
    parser_init(&parser, rule_expression,
                /*ignore_newline=*/true);
    SyntaxTree *stree = parser_parse(&parser, &tokens);
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
    TokenArray_finalize(&tokens);
  }

  // Below code not necessary as the program will immediately free all memory
  // upon exit.

  // file_info_delete(file);
  // token_finalize_all();
  // global_string_intern_pool_finalize();
}