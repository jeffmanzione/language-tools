load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@language_tools//lang/lexer:lexer_builder.bzl", "lexer_builder")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "lexer_helper",
    srcs = ["lexer_helper.c"],
    hdrs = ["lexer_helper.h"],
    deps = [
        "@file_utils//util:string",
        "@memory_wrapper//alloc",
    ],
)

cc_library(
    name = "lexer_builder",
    srcs = ["lexer_builder.c"],
    hdrs = ["lexer_builder.h"],
    deps = [
        ":lexer_helper",
        "@c_data_structures//struct:alist",
        "@file_utils//util:string",
        "@file_utils//util/file:file_info",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

cc_binary(
    name = "lexer_builder_main",
    srcs = ["lexer_builder_main.c"],
    deps = [
        ":lexer_builder",
        "@file_utils//util/file:file_info",
        "@file_utils//util/file:file_util",
        "@memory_wrapper//alloc/arena:intern",
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
        "@c_data_structures//struct:q",
        "@file_utils//util/file:file_info",
        "@file_utils//util/file:sfile",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

cc_library(
    name = "token",
    srcs = ["token.c"],
    hdrs = ["token.h"],
    deps = [
        "@memory_wrapper//alloc/arena",
        "@memory_wrapper//alloc/arena:intern",
        "@memory_wrapper//debug",
    ],
)
