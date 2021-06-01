def _lexer_builder_impl(ctx):
    lexer_builder_output = ctx.actions.declare_file(ctx.label.name + ".c")
    args = ctx.actions.args()
    args.add_all([lexer_builder_output, ctx.file.symbols, ctx.file.keywords, ctx.file.comments])
    ctx.actions.run(
        mnemonic = "LexerBuilder",
        executable = ctx.executable.lexer_builder_main,
        arguments = [args],
        inputs = depset([ctx.file.symbols, ctx.file.keywords, ctx.file.comments]),
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
        "lexer_builder_main": attr.label(
            default = Label("//lang/lexer:lexer_builder_main"),
            executable = True,
            allow_single_file = True,
            cfg = "exec",
        ),
    },
)

def lexer_builder(name, symbols, keywords, comments):
    return _lexer_builder(
        name = name,
        symbols = symbols,
        keywords = keywords,
        comments = comments,
    )
