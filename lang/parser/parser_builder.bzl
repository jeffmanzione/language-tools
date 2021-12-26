def _parser_builder_impl(ctx):
    out_file_name = ctx.label.name.replace("_h", "").replace("_c", "") + (".h" if ctx.attr.header else ".c")
    parser_builder_output = ctx.actions.declare_file(out_file_name)
    program_config = "header" if ctx.attr.header else "source"
    args = ctx.actions.args()
    args.add_all([ctx.file.rules, program_config, parser_builder_output])
    if not ctx.attr.header:
        h_file = "%s/%s" % (ctx.label.package, out_file_name.replace(".c", ".h"))
        lexer_h_file = "%s/%s.h" % (ctx.attr.lexer.label.package, ctx.attr.lexer.label.name)
        args.add_all([h_file, lexer_h_file])
    ctx.actions.run(
        mnemonic = "ParserBuilder",
        executable = ctx.executable.parser_builder_main,
        arguments = [args],
        inputs = depset([ctx.file.rules]),
        outputs = [parser_builder_output],
    )
    return [
        DefaultInfo(
            files = depset([parser_builder_output]),
        ),
    ]

_parser_builder = rule(
    implementation = _parser_builder_impl,
    attrs = {
        "header": attr.bool(
            default = False,
            doc = "should output header.",
        ),
        "rules": attr.label(
            allow_single_file = True,
            doc = "rules txt file.",
        ),
        "lexer": attr.label(),
        "parser_builder_main": attr.label(
            default = Label("@language_tools//lang/parser/production_parser:production_parser_main"),
            executable = True,
            allow_single_file = True,
            cfg = "exec",
        ),
    },
)

def parser_builder(name, rules, lexer):
    _parser_builder(
        name = "%s_h" % name,
        header = True,
        rules = rules,
        lexer = lexer,
    )
    _parser_builder(
        name = "%s_c" % name,
        rules = rules,
        lexer = lexer,
    )
    return native.cc_library(
        name = name,
        hdrs = [":%s_h" % name],
        srcs = [":%s_c" % name],
        deps = [
            lexer,
            "@language_tools//lang/parser",
            "@c_data_structures//struct:alist",
        ],
    )
