load("@rules_cc//cc:defs.bzl", "cc_binary")

package(
    default_visibility = ["//visibility:public"],
)

cc_binary(
    name = "production_parser",
    srcs = ["production_parser.c"],
    deps = [
        "//lang/lexer:token",
        "//lang/parser/production_lexer",
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
