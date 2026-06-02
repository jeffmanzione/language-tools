load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

# load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("//language-tools/parser:parser_builder.bzl", "parser_builder")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "parser_builder",
    srcs = ["parser_builder.c"],
    hdrs = ["parser_builder.h"],
    deps = [
        "//language-tools:intern",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_c_data_structures//c-data-structures:maplike",
        "@jeffmanzione_file_utils//file-utils:string_utils",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.c"],
    hdrs = ["parser.h"],
    deps = [
        "//language-tools/lexer:token",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_rzalloc//rzalloc",
    ],
)

cc_binary(
    name = "parser_test",
    srcs = ["parser_test.c"],
    deps = [
        ":parser",
        ":test_parser",
        "//language-tools:intern",
        "//language-tools/lexer:test_lexer",
        "//language-tools/lexer:token",
        "@jeffmanzione_file_utils//file-utils:file_info",
        "@jeffmanzione_file_utils//file-utils:sfile",
    ],
)

parser_builder(
    name = "test_parser",
    lexer = "//language-tools/lexer:test_lexer",
    rules = "testdata/rules.txt",
)
