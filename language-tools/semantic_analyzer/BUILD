load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(
    default_visibility = ["//language-tools:internal"],
)

cc_library(
    name = "expression_tree",
    srcs = ["expression_tree.c"],
    hdrs = ["expression_tree.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//language-tools/lexer:token",
        "//language-tools/parser",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_c_data_structures//c-data-structures:maplike",
    ],
)

cc_library(
    name = "semantic_analyzer",
    srcs = ["semantic_analyzer.c"],
    hdrs = ["semantic_analyzer.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":expression_tree",
        "//language-tools/parser",
    ],
)
