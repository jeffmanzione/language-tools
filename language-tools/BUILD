load(
    "@rules_cc//cc:defs.bzl",
    "cc_library",
)

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "intern",
    srcs = ["intern.c"],
    hdrs = ["intern.h"],
    deps = [
        "@jeffmanzione_intern//intern",
    ],
)
