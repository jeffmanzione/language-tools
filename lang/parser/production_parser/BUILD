load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = ["//visibility:public"],
)

cc_binary(
    name = "production_parser_test",
    srcs = ["production_parser_test.c"],
    deps = [
        ":production_rules",
        "//lang/lexer:token",
        "//lang/parser/production_lexer",
        "//lang/semantic_analyzer",
        "//lang/semantic_analyzer:expression_tree",
        "@c_data_structures//struct:q",
        "@file_utils//util/file:file_info",
        "@memory_wrapper//alloc/arena:intern",
    ],
)

cc_binary(
    name = "production_parser_generator",
    srcs = ["production_parser_generator.c"],
    deps = [
        "//lang/parser:parser_builder",
        "//lang/parser/production_lexer",
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
    cmd = "./$(location :production_parser_generator) source lang/parser/production_parser/production_rules.h lang/parser/production_lexer/production_lexer.h > $@",
)

cc_library(
    name = "production_rules",
    srcs = [":production_rules_c"],
    hdrs = [":production_rules_h"],
    deps = [
        "//lang/lexer:token",
        "//lang/parser",
        "//lang/parser/production_lexer",
    ],
)
