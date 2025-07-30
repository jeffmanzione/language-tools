load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("//lang/parser:parser_builder.bzl", "parser_builder")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "parser_builder",
    srcs = ["parser_builder.c"],
    hdrs = ["parser_builder.h"],
    deps = [
        "@c_data_structures//struct:alist",
        "@file_utils//util:string",
        "@memory_wrapper//alloc",
        "@memory_wrapper//alloc/arena:intern",
        "@memory_wrapper//debug",
        "@memory_wrapper//struct:map",
        "@memory_wrapper//struct:struct_defaults",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.c"],
    hdrs = ["parser.h"],
    deps = [
        "//lang/lexer:token",
        "@c_data_structures//struct:alist",
        "@c_data_structures//struct:q",
        "@memory_wrapper//alloc/arena",
    ],
)

cc_binary(
    name = "parser_test",
    srcs = ["parser_test.c"],
    deps = [
        ":parser",
        ":test_parser",
        "//lang/lexer:test_lexer",
        "//lang/lexer:token",
        "@c_data_structures//struct:q",
        "@file_utils//util/file:file_info",
        "@file_utils//util/file:sfile",
        "@memory_wrapper//alloc",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

parser_builder(
    name = "test_parser",
    lexer = "//lang/lexer:test_lexer",
    rules = "testdata/rules.txt",
)
