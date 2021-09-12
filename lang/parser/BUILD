load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "parser_builder",
    srcs = ["parser_builder.c"],
    hdrs = ["parser_builder.h"],
    deps = [
        "@c_data_structures//struct:alist",
        "@memory_wrapper//alloc",
    ],
)

cc_binary(
    name = "parser_test",
    srcs = ["parser_test.c"],
    deps = [
        ":parser_builder",
        "//lang/lexer:test_lexer",
    ],
)
