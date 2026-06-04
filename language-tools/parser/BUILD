load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(
    default_visibility = ["//language-tools:internal"],
)

cc_library(
    name = "parser_builder",
    srcs = ["parser_builder.c"],
    hdrs = ["parser_builder.h"],
    visibility = ["//visibility:public"],
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
    visibility = ["//visibility:public"],
    deps = [
        "//language-tools/lexer:token",
        "@jeffmanzione_c_data_structures//c-data-structures:arraylike",
        "@jeffmanzione_rzalloc//rzalloc",
    ],
)
