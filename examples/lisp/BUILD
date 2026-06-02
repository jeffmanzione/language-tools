load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("//language-tools/lexer:lexer_builder.bzl", "lexer_builder")
load("//language-tools/parser:parser_builder.bzl", "parser_builder")

package(
    default_visibility = ["//visibility:public"],
)

lexer_builder(
    name = "lisp_lexer",
    comments = "comments.txt",
    enum_prefix = "Lisp",
    fn_prefix = "lisp_",
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
        "//language-tools/lexer:token",
        "//language-tools/parser/production_lexer",
        "//language-tools/semantic_analyzer",
        "//language-tools/semantic_analyzer:expression_tree",
    ],
)

cc_binary(
    name = "lisp_cli",
    srcs = ["lisp_cli.c"],
    deps = [
        ":lisp_lexer",
        ":lisp_parser",
        ":lisp_semantics",
        "//language-tools:intern",
        "//language-tools/lexer:token",
        "//language-tools/parser",
        "//language-tools/parser/production_lexer",
        "//language-tools/semantic_analyzer",
        "//language-tools/semantic_analyzer:expression_tree",
        "@jeffmanzione_file_utils//file-utils:file_info",
        "@jeffmanzione_file_utils//file-utils:sfile",
    ],
)
