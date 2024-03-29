load("@language_tools//lang/lexer:lexer_builder.bzl", "lexer_builder")
load("@language_tools//lang/parser:parser_builder.bzl", "parser_builder")

package(
    default_visibility = ["//visibility:public"],
)

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

cc_library(
    name = "lisp_semantics",
    srcs = ["semantics.c"],
    hdrs = ["semantics.h"],
    deps = [
        ":lisp_parser",
        "@language_tools//lang/lexer:token",
        "@language_tools//lang/parser/production_lexer",
        "@language_tools//lang/semantic_analyzer",
        "@language_tools//lang/semantic_analyzer:expression_tree",
    ],
)

cc_binary(
    name = "lisp_cli",
    srcs = ["lisp_cli.c"],
    deps = [
        ":lisp_lexer",
        ":lisp_parser",
        ":lisp_semantics",
        "@c_data_structures//struct:q",
        "@file_utils//util/file:file_info",
        "@file_utils//util/file:sfile",
        "@language_tools//lang/lexer:token",
        "@language_tools//lang/parser",
        "@language_tools//lang/parser/production_lexer",
        "@language_tools//lang/semantic_analyzer",
        "@language_tools//lang/semantic_analyzer:expression_tree",
        "@memory_wrapper//alloc",
        "@memory_wrapper//alloc/arena:intern",
        "@memory_wrapper//debug",
    ],
)
