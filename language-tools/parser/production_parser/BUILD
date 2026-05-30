load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = ["//visibility:public"],
)

cc_binary(
    name = "production_parser_main",
    srcs = ["production_parser_main.c"],
    deps = [
        ":production_parser_semantics",
        ":production_rules",
        "//language-tools/lexer:token",
        "//language-tools/parser:parser_builder",
        "//language-tools/parser/production_lexer",
        "//language-tools/semantic_analyzer",
        "//language-tools/semantic_analyzer:expression_tree",
        "@c_data_structures//struct:q",
        "@file_utils//util/file:file_info",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

cc_library(
    name = "production_parser_semantics",
    srcs = ["production_parser_semantics.c"],
    hdrs = ["production_parser_semantics.h"],
    deps = [
        ":production_rules",
        "//language-tools/lexer:token",
        "//language-tools/parser/production_lexer",
        "//language-tools/semantic_analyzer",
        "//language-tools/semantic_analyzer:expression_tree",
    ],
)

cc_binary(
    name = "production_parser_generator",
    srcs = ["production_parser_generator.c"],
    deps = [
        "//language-tools/parser:parser_builder",
        "//language-tools/parser/production_lexer",
        "@memory_wrapper//alloc",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

genrule(
    name = "production_rules_h",
    srcs = [":production_parser_generator"],
    outs = ["production_rules.h"],
    cmd = "./$(location :production_parser_generator) header > $@",
)

genrule(
    name = "production_rules_c",
    srcs = [":production_parser_generator"],
    outs = ["production_rules.c"],
    cmd = "./$(location :production_parser_generator) source language-tools/parser/production_parser/production_rules.h language-tools/parser/production_lexer/production_lexer.h > $@",
)

cc_library(
    name = "production_rules",
    srcs = [":production_rules_c"],
    hdrs = [":production_rules_h"],
    deps = [
        "//language-tools/lexer:token",
        "//language-tools/parser",
        "//language-tools/parser/production_lexer",
    ],
)
