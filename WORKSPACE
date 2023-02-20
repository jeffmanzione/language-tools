load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "c_data_structures",
    branch = "main",
    remote = "https://github.com/jeffmanzione/c-data-structures.git",
)

git_repository(
    name = "memory_wrapper",
    branch = "master",
    remote = "https://github.com/jeffmanzione/memory-wrapper.git",
)

git_repository(
    name = "file_utils",
    branch = "main",
    remote = "https://github.com/jeffmanzione/file-utils.git",
)

# Self-reference to support example build rule usages.
git_repository(
    name = "language_tools",
    branch = "main",
    remote = "https://github.com/jeffmanzione/language-tools.git",
)
