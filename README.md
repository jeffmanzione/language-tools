# language-tools

A C library for creating programming languages.

## Code generation

The library generates code for lexers, parsers, and semantic analyzers through [bazel](https://bazel.build) rules.

## Example

An example toy LISP cli using this library can be found [here](https://github.com/jeffreymanzione/language-tools/tree/main/examples/lisp).

To create the code generation for your language, define bazel rules for your lexer and parser.

`BUILD`:

```starlark
load("@language_tools//lang/lexer:lexer_builder.bzl", "lexer_builder")
load("@language_tools//lang/parser:parser_builder.bzl", "parser_builder")

lexer_builder(
    name = "lisp_lexer",
    comments = "comments.txt",
    keywords = "keywords.txt",
    strings = "strings.txt",
    symbols = "symbols.txt",
)

parser_builder(
    name = "lisp_parser",
    lexer = ":lisp_lexer",
    rules = "rules.txt",
)
```

### Defintions

The `.txt` files define your language:

* `comments.txt`: Defines which sequences start and end comments.
  Example defining `;` to open and `\n` to close a comment:
  
  ```txt
  COMMENT_LINE,;,\n
  ```

* `keywords.txt`: Defines which sequences are keywords.
  Example:
  
  ```txt
  KEYWORD_AND,and
  KEYWORD_OR,or
  KEYWORD_NOT,not
  KEYWORD_IF,if
  ```

* `strings.txt`: Defines which sequences start and end strings.

  Example:
  
  ```txt
  STRING_SINGLEQUOTE,','
  ```

* `symbols.txt`: Defines which sequences are symbols.

  Example:
  
  ```txt
  SYMBOL_LPAREN,(
  SYMBOL_RPAREN,)
  SYMBOL_PLUS,+
  SYMBOL_MINUS,-
  SYMBOL_STAR,*
  SYMBOL_FSLASH,/
  ```

* `rules.txt`: Defines the syntax of the language.

  Example rules for the LISP language:

  ```txt
  function ->
    OR(
      token:KEYWORD_AND,
      token:KEYWORD_OR,
      token:KEYWORD_NOT,
      token:KEYWORD_IF,
      token:SYMBOL_PLUS,
      token:SYMBOL_MINUS,
      token:SYMBOL_STAR,
      token:SYMBOL_FSLASH
    );

  expression ->
    OR(
      rule:expression_function,
      token:TOKEN_INTEGER,
      token:TOKEN_FLOATING
    );

  expression_function ->
    AND(
      token:SYMBOL_LPAREN,
      rule:function,
      rule:expression_function_items,
      token:SYMBOL_RPAREN
    );

  expression_function_items -> LIST(E, rule:expression);
  ```

### Semantic Analysis

You can translate the generated syntax trees for your code into data structures.

```c

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
  ...
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
```

### Using your code

```c
// Creates an empty queue.
Q tokens;
Q_init(&tokens);
// Lexes/tokenizes a file into tokens.
lexer_tokenize(file, &tokens);

// Parse your tokens into a sytax tree.
Parser parser;
parser_init(&parser, rule_expression,
            /*ignore_newline=*/false);
SyntaxTree *stree = parser_parse(&parser, &tokens);

// Map the syntax tree to data structures that you can use.
SemanticAnalyzer analyzer;
semantic_analyzer_init(&analyzer, init_semantics);
ExpressionTree *etree = semantic_analyzer_populate(&analyzer, stree);

// Use your expression as you see fit.
double result = evaluate_lisp_expression(etree, stdout);
```
