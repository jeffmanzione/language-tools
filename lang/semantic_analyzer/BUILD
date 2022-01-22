load("@rules_cc//cc:defs.bzl", "cc_library")

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "expression_tree",
    hdrs = ["expression_tree.h"],
    deps = [
        "//lang/lexer:token",
        "//lang/parser",
        "@c_data_structures//struct:alist",
        "@memory_wrapper//alloc",
        "@memory_wrapper//debug",
        "@memory_wrapper//struct:map",
        "@memory_wrapper//struct:struct_defaults",
    ],
)

cc_library(
    name = "semantic_analyzer",
    srcs = ["semantic_analyzer.c"],
    hdrs = ["semantic_analyzer.h"],
    deps = [
        ":expression_tree",
        "//lang/parser",
        "@memory_wrapper//debug",
        "@memory_wrapper//struct:map",
        "@memory_wrapper//struct:struct_defaults",
    ],
)
