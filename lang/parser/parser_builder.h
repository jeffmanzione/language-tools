#ifndef LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_
#define LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_

#include <stdio.h>

typedef struct _ParserBuilder ParserBuilder;
typedef struct _Production Production;
typedef const char *(*TokenToStringFn)(int);
typedef int (*StringToTokenFn)(const char *);

ParserBuilder *parser_builder_create();
void parser_builder_delete(ParserBuilder *pb);
void parser_builder_write_c_file(ParserBuilder *pb, const char h_file_path[],
                                 const char lexer_h_file_path[], FILE *file);
void parser_builder_write_h_file(ParserBuilder *pb, FILE *file);
void parser_builder_set_root(ParserBuilder *pb, Production *p);
void parser_builder_rule(ParserBuilder *pb, const char rule_name[],
                         Production *p);

// clang-format off
#define GET_FUNC(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
#define or2(p1, p2) __or(2, p1, p2)
#define or3(p1, p2, p3) __or(3, p1, p2, p3)
#define or4(p1, p2, p3, p4) __or(4, p1, p2, p3, p4)
#define or5(p1, p2, p3, p4, p5) __or(5, p1, p2, p3, p4, p5)
#define or6(p1, p2, p3, p4, p5, p6) __or(6, p1, p2, p3, p4, p5, p6)
#define or7(p1, p2, p3, p4, p5, p6, p7) __or(7, p1, p2, p3, p4, p5, p6, p7)
#define or8(p1, p2, p3, p4, p5, p6, p7, p8)                                     \
  __or(8, p1, p2, p3, p4, p5, p6, p7, p8)
#define or9(p1, p2, p3, p4, p5, p6, p7, p8, p9)                                 \
  __or(9, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define or(...)                                                                 \
  GET_FUNC(__VA_ARGS__, or9, or8, or7, or6, or5, or4, or3, or2)(__VA_ARGS__)
#define and2(p1, p2) __and(2, p1, p2)
#define and3(p1, p2, p3) __and(3, p1, p2, p3)
#define and4(p1, p2, p3, p4) __and(4, p1, p2, p3, p4)
#define and5(p1, p2, p3, p4, p5) __and(5, p1, p2, p3, p4, p5)
#define and6(p1, p2, p3, p4, p5, p6)  __and(6, p1, p2, p3, p4, p5, p6)
#define and7(p1, p2, p3, p4, p5, p6, p7) __and(7, p1, p2, p3, p4, p5, p6, p7)
#define and8(p1, p2, p3, p4, p5, p6, p7, p8)                                    \
  __and(8, p1, p2, p3, p4, p5, p6, p7, p8)
#define and9(p1, p2, p3, p4, p5, p6, p7, p8, p9)                                \
  __and(9, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define and(...)                                                                \
  GET_FUNC(__VA_ARGS__, and9, and8, and7, and6, and5, and4, and3, and2)         \
      (__VA_ARGS__)
// clang-format on

Production *__or(int arg_count, ...);
Production *__and(int arg_count, ...);

Production *rule(const char rule_name[]);
Production *token(const char token[]);
Production *newline();
Production *optional(Production *p_child);
Production *line(Production *p);
Production *epsilon();

Production *production_and();
Production *production_or();
void production_add_child(Production *parent, Production *child);

void parser_builder_print(ParserBuilder *pb, FILE *out);

#endif /* LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_ */