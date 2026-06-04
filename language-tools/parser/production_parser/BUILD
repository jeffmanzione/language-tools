load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(
    default_visibility = ["//language-tools:internal"],
)

cc_binary(
    name = "production_parser_main",
    srcs = ["production_parser_main.c"],
    visibility = ["//visibility:public"],
    deps = [
        ":production_parser_semantics",
        ":production_rules",
        "//language-tools:intern",
        "//language-tools/lexer:token",
        "//language-tools/parser:parser_builder",
        "//language-tools/parser/production_lexer",
        "//language-tools/semantic_analyzer",
        "//language-tools/semantic_analyzer:expression_tree",
        "@jeffmanzione_file_utils//file-utils:file_info",
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
        "//language-tools:intern",
        "//language-tools/parser:parser_builder",
        "//language-tools/parser/production_lexer",
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
