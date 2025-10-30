#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// 前向声明 AST 节点结构体，ast.h 将会定义它
struct ASTNode;

// 定义扫描器状态结构体
// (综合 1.3, 2.1, 4.2, 4.4 节)
typedef struct {
    const unsigned char *cursor; // 当前扫描位置
    const unsigned char *marker; // re2c 用于回溯的标记
    const unsigned char *limit;  // 输入缓冲区的末尾
    int line;                  // 当前行号
    
    // ASI 相关的状态标志
    bool line_terminator_seen; // 紧邻当前 token 之前是否看到了换行符
    bool restrict_new_line;    // 是否处于受限产生式中（如 return 之后）
} Scanner;

/*
 * YYSTYPE 是由 Bison 在 parser.tab.h 中定义的
 * 我们不能在这里定义它，但包含它的文件 (lexer.re, parser.y)
 * 需要知道这个结构体。
 *
 * lexer.re 将包含 parser.tab.h
 * parser.y 将定义 %union，它会创建 YYSTYPE
 */

// 声明由 re2c 实现的词法分析函数
// YYSTYPE 将由 parser.tab.h 定义
typedef union YYSTYPE YYSTYPE;
int real_yylex(YYSTYPE *yylval, Scanner *scanner);

// 声明错误报告函数
void yyerror(const char *s);

#endif // COMMON_H