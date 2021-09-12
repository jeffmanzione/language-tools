#ifndef LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_
#define LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_

#include <stdio.h>

typedef struct _ParserBuilder ParserBuilder;
typedef struct _ProductionBuilder ProductionBuilder;

ParserBuilder *parser_builder_create();
void parser_builder_delete(ParserBuilder *pb);
void parser_builder_write_c_file(ParserBuilder *pb, FILE *file);
void parser_builder_write_h_file(ParserBuilder *pb, FILE *file);

void production_builder_delete(ProductionBuilder *pb);

// clang-format off
#define GET_FUNC(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
#define or2(p1, p2) __or(2, p1, ep2)
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

ProductionBuilder *__or(int arg_count, ...);
ProductionBuilder *__and(int arg_count, ...);

ProductionBuilder *token(int token);
ProductionBuilder *newline();
ProductionBuilder *line(ProductionBuilder *pb);
ProductionBuilder *epsilon();

#endif /* LANGUAGE_TOOLS_LANG_PARSER_PARSER_BUILDER_H_ */