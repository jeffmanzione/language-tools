load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(
    default_visibility = [":internal"],
)

package_group(
    name = "internal",
    packages = ["//language-tools/..."],
)

cc_library(
    name = "intern",
    srcs = ["intern.c"],
    hdrs = ["intern.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@jeffmanzione_intern//intern",
    ],
)
