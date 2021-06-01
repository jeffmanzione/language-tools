load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//lang/lexer:lexer_builder.bzl", "lexer_builder")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "lexer_helper",
    srcs = ["lexer_helper.c"],
    hdrs = ["lexer_helper.h"],
    deps = [],
)

cc_library(
    name = "lexer_builder",
    srcs = ["lexer_builder.c"],
    hdrs = ["lexer_builder.h"],
    deps = [
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
    name = "test",
    comments = "testdata/comments.txt",
    keywords = "testdata/keywords.txt",
    symbols = "testdata/symbols.txt",
)
