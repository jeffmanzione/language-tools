load("@rules_cc//cc:cc_library.bzl", "cc_library")

def _lexer_builder_impl(ctx):
    out_file_name = ctx.label.name.replace("_h", "").replace("_c", "") + (".h" if ctx.attr.header else ".c")
    lexer_builder_output = ctx.actions.declare_file(out_file_name)
    program_config = "header" if ctx.attr.header else lexer_builder_output.short_path.replace(".c", ".h")
    args = ctx.actions.args()
    args.add_all([
        program_config,
        lexer_builder_output.path,
        ctx.file.symbols,
        ctx.file.keywords,
        ctx.file.comments,
        ctx.file.strings,
        ctx.attr.fn_prefix,
        ctx.attr.enum_prefix,
    ])
    ctx.actions.run(
        mnemonic = "LexerBuilder",
        executable = ctx.executable.lexer_builder_main,
        arguments = [args],
        inputs = depset([
            ctx.file.symbols,
            ctx.file.keywords,
            ctx.file.comments,
            ctx.file.strings,
        ]),
        outputs = [lexer_builder_output],
    )
    return [
        DefaultInfo(
            files = depset([lexer_builder_output]),
        ),
    ]

_lexer_builder = rule(
    implementation = _lexer_builder_impl,
    attrs = {
        "header": attr.bool(
            default = False,
            doc = "should output header.",
        ),
        "fn_prefix": attr.string(
            default = "",
            doc = "prefix for generated lexer functions.",
        ),
        "enum_prefix": attr.string(
            default = "",
            doc = "prefix for generated lexer enums.",
        ),
        "symbols": attr.label(
            allow_single_file = True,
            doc = "symbols txt file.",
        ),
        "keywords": attr.label(
            allow_single_file = True,
            doc = "keywords txt file.",
        ),
        "comments": attr.label(
            allow_single_file = True,
            doc = "comments txt file.",
        ),
        "strings": attr.label(
            allow_single_file = True,
            doc = "strings txt file.",
        ),
        "lexer_builder_main": attr.label(
            default = Label("//language-tools/lexer:lexer_builder_main"),
            executable = True,
            allow_single_file = True,
            cfg = "exec",
        ),
    },
)

def lexer_builder(
        name,
        symbols,
        keywords,
        comments,
        strings,
        fn_prefix = None,
        enum_prefix = None):
    _lexer_builder(
        name = "%s_h" % name,
        header = True,
        symbols = symbols,
        keywords = keywords,
        comments = comments,
        strings = strings,
        fn_prefix = fn_prefix,
        enum_prefix = enum_prefix,
    )
    _lexer_builder(
        name = "%s_c" % name,
        symbols = symbols,
        keywords = keywords,
        comments = comments,
        strings = strings,
        fn_prefix = fn_prefix,
        enum_prefix = enum_prefix,
    )
    return cc_library(
        name = name,
        hdrs = [":%s_h" % name],
        srcs = [":%s_c" % name],
        deps = [
            "//language-tools/lexer:lexer_helper",
            "//language-tools/lexer:token",
            "@jeffmanzione_file_utils//file-utils:string_utils",
            "@jeffmanzione_file_utils//file-utils:file_info",
        ],
    )
