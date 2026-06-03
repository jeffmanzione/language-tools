load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("//language-tools/lexer:lexer_builder.bzl", "lexer_builder")

package(
    default_visibility = ["//language-tools:internal"],
)

cc_library(
    name = "lexer_helper",
    srcs = ["lexer_helper.c"],
    hdrs = ["lexer_helper.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "lexer_builder",
    srcs = ["lexer_builder.c"],
    hdrs = ["lexer_builder.h"],
    deps = [
        ":lexer_helper",
        "//language-tools:intern",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_file_utils//file-utils:file_info",
        "@jeffmanzione_file_utils//file-utils:string_utils",
    ],
)

cc_binary(
    name = "lexer_builder_main",
    srcs = ["lexer_builder_main.c"],
    visibility = ["//visibility:public"],
    deps = [
        ":lexer_builder",
        "//language-tools:intern",
        "@jeffmanzione_file_utils//file-utils:file_info",
        "@jeffmanzione_file_utils//file-utils:file_utils",
    ],
)

lexer_builder(
    name = "test_lexer",
    comments = "testdata/comments.txt",
    keywords = "testdata/keywords.txt",
    strings = "testdata/strings.txt",
    symbols = "testdata/symbols.txt",
)

cc_binary(
    name = "lexer_test",
    srcs = ["lexer_test.c"],
    deps = [
        ":test_lexer",
        ":token",
        "//language-tools:intern",
        "@jeffmanzione_file_utils//file-utils:file_info",
        "@jeffmanzione_file_utils//file-utils:sfile",
    ],
)

cc_library(
    name = "token",
    srcs = ["token.c"],
    hdrs = ["token.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//language-tools:intern",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_rzalloc//rzalloc",
    ],
)
