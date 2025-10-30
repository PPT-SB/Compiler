// src/lexer.re
#include <stdio.h>
#include <string.h> // for strndup
#include <stdbool.h>
#include "common.h" // 包含 Scanner 定义
#include "ast.h"    // 包含 AST 定义
#include "parser.tab.h" // 包含 Bison 生成的词法单元定义

// yylex 函数，Bison 将调用此函数获取下一个词法单元
int real_yylex(YYSTYPE *yylval, Scanner *scanner) {
    const unsigned char *yyt1; // re2c 用于捕获文本的指针

    // C 宏定义必须在 re2c 块之外
    #define YYACTION_RESET_ASI scanner->line_terminator_seen = false;

yyc_start: // re2c 规则的起始标签

    /*!re2c
        // --- 1. CONFIGS (必须在最前面) ---
        re2c:yyfill:enable = 0;
        re2c:api:style = free-form;
        re2c:define:YYCTYPE = "unsigned char";
        re2c:define:YYCURSOR = "scanner->cursor";
        re2c:define:YYMARKER = "scanner->marker";
        re2c:define:YYLIMIT = "scanner->limit";
        re2c:encoding:utf8 = 1; // 启用 UTF-8 以识别 \p
        re2c:eof = 0;

        // --- 2. RULES (已移除所有命名定义，全部内联) ---

        // 空白与注释 (来自 2.2 节)
        [\t\v\f\u0020\u00A0\uFEFF]+ { 
            goto yyc_start; 
        }
        "//" [^\r\n]* { 
            goto yyc_start; 
        }
        "/*" ([^*] | "*" [^/])* "*/" { 
            goto yyc_start; 
        }

        // 行终止符 (来自 4.4 节)
        [\r\n\u2028\u2029]+ {
            if (scanner->restrict_new_line) {
                // 受限产生式规则被触发！
                scanner->restrict_new_line = false; // 重置状态
                scanner->line_terminator_seen = true; // 报告看到了换行
                return SEMICOLON;
            }
            scanner->line++;
            scanner->line_terminator_seen = true;
            goto yyc_start; // 继续扫描，不返回 token
        }
        
        // 标点符号与关键字 (来自 2.2 节)
        "{"         { YYACTION_RESET_ASI; return LBRACE; }
        "}"         { YYACTION_RESET_ASI; return RBRACE; }
        "("         { YYACTION_RESET_ASI; return LPAREN; }
        ")"         { YYACTION_RESET_ASI; return RPAREN; }
        "["         { YYACTION_RESET_ASI; return LBRACK; }
        "]"         { YYACTION_RESET_ASI; return RBRACK; }
        "."         { YYACTION_RESET_ASI; return DOT; }
        ";"         { YYACTION_RESET_ASI; return SEMICOLON; }
        ","         { YYACTION_RESET_ASI; return COMMA; }
        "<"         { YYACTION_RESET_ASI; return LT; }
        ">"         { YYACTION_RESET_ASI; return GT; }
        "<="        { YYACTION_RESET_ASI; return LE; }
        ">="        { YYACTION_RESET_ASI; return GE; }
        "=="        { YYACTION_RESET_ASI; return EQ; }
        "!="        { YYACTION_RESET_ASI; return NE; }
        "==="       { YYACTION_RESET_ASI; return STRICT_EQ; }
        "!=="       { YYACTION_RESET_ASI; return STRICT_NE; }
        "+"         { YYACTION_RESET_ASI; return PLUS; }
        "-"         { YYACTION_RESET_ASI; return MINUS; }
        "*"         { YYACTION_RESET_ASI; return MUL; }
        "%"         { YYACTION_RESET_ASI; return MOD; }
        "**"        { YYACTION_RESET_ASI; return POWER; }
        "++"        { YYACTION_RESET_ASI; return INC; }
        "--"        { YYACTION_RESET_ASI; return DEC; }
        "<<"        { YYACTION_RESET_ASI; return LSHIFT; }
        ">>"        { YYACTION_RESET_ASI; return RSHIFT; }
        ">>>"       { YYACTION_RESET_ASI; return URSHIFT; }
        "&"         { YYACTION_RESET_ASI; return BIT_AND; }
        "|"         { YYACTION_RESET_ASI; return BIT_OR; }
        "^"         { YYACTION_RESET_ASI; return BIT_XOR; }
        "!"         { YYACTION_RESET_ASI; return NOT; }
        "~"         { YYACTION_RESET_ASI; return BIT_NOT; }
        "&&"        { YYACTION_RESET_ASI; return LOGICAL_AND; }
        "||"        { YYACTION_RESET_ASI; return LOGICAL_OR; }
        "??"        { YYACTION_RESET_ASI; return NULLISH_COALESCING; }
        "?"         { YYACTION_RESET_ASI; return CONDITIONAL; }
        ":"         { YYACTION_RESET_ASI; return COLON; }
        "="         { YYACTION_RESET_ASI; return ASSIGN; }
        "+="        { YYACTION_RESET_ASI; return ADD_ASSIGN; }
        "-="        { YYACTION_RESET_ASI; return SUB_ASSIGN; }
        "*="        { YYACTION_RESET_ASI; return MUL_ASSIGN; }
        "**="       { YYACTION_RESET_ASI; return POWER_ASSIGN; }
        "=>"        { YYACTION_RESET_ASI; return ARROW; }
        "..."       { YYACTION_RESET_ASI; return SPREAD; }
        "break"     { YYACTION_RESET_ASI; return BREAK; }
        "case"      { YYACTION_RESET_ASI; return CASE; }
        "catch"     { YYACTION_RESET_ASI; return CATCH; }
        "class"     { YYACTION_RESET_ASI; return CLASS; }
        "const"     { YYACTION_RESET_ASI; return CONST; }
        "continue"  { YYACTION_RESET_ASI; return CONTINUE; }
        "debugger"  { YYACTION_RESET_ASI; return DEBUGGER; }
        "default"   { YYACTION_RESET_ASI; return DEFAULT; }
        "delete"    { YYACTION_RESET_ASI; return DELETE; }
        "do"        { YYACTION_RESET_ASI; return DO; }
        "else"      { YYACTION_RESET_ASI; return ELSE; }
        "export"    { YYACTION_RESET_ASI; return EXPORT; }
        "extends"   { YYACTION_RESET_ASI; return EXTENDS; }
        "finally"   { YYACTION_RESET_ASI; return FINALLY; }
        "for"       { YYACTION_RESET_ASI; return FOR; }
        "function"  { YYACTION_RESET_ASI; return FUNCTION; }
        "if"        { YYACTION_RESET_ASI; return IF; }
        "import"    { YYACTION_RESET_ASI; return IMPORT; }
        "in"        { YYACTION_RESET_ASI; return IN; }
        "instanceof" { YYACTION_RESET_ASI; return INSTANCEOF; }
        "new"       { YYACTION_RESET_ASI; return NEW; }
        "return"    { YYACTION_RESET_ASI; return RETURN; }
        "super"     { YYACTION_RESET_ASI; return SUPER; }
        "switch"    { YYACTION_RESET_ASI; return SWITCH; }
        "this"      { YYACTION_RESET_ASI; return THIS; }
        "throw"     { YYACTION_RESET_ASI; return THROW; }
        "try"       { YYACTION_RESET_ASI; return TRY; }
        "typeof"    { YYACTION_RESET_ASI; return TYPEOF; }
        "var"       { YYACTION_RESET_ASI; return VAR; }
        "void"      { YYACTION_RESET_ASI; return VOID; }
        "while"     { YYACTION_RESET_ASI; return WHILE; }
        "with"      { YYACTION_RESET_ASI; return WITH; }
        "yield"     { YYACTION_RESET_ASI; return YIELD; }
        "let"       { YYACTION_RESET_ASI; return LET; }
        "static"    { YYACTION_RESET_ASI; return STATIC; }
        "enum"      { YYACTION_RESET_ASI; return ENUM; }
        "await"     { YYACTION_RESET_ASI; return AWAIT; }
        "true"      { YYACTION_RESET_ASI; return TRUE_LITERAL; }
        "false"     { YYACTION_RESET_ASI; return FALSE_LITERAL; }
        "null"      { YYACTION_RESET_ASI; return NULL_LITERAL; }

        // --- 规则：已内联 (之前导致错误的部分) ---

        // 标识符 (Identifier)
        @yyt1 [a-zA-Z_][a-zA-Z0-9_]* {
            YYACTION_RESET_ASI;
            size_t len = scanner->cursor - yyt1;
            yylval->str_val = strndup((const char*)yyt1, len);
            return IDENTIFIER;
        }

        // 数值字面量 (HexIntegerLiteral)
        @yyt1 "0" [xX] [0-9a-fA-F]+ {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return NUMERIC_LITERAL;
        }
        // (OctalIntegerLiteral)
        @yyt1 "0" [oO] [0-7]+ {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return NUMERIC_LITERAL;
        }
        // (BinaryIntegerLiteral)
        @yyt1 "0" [bB] [01]+ {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return NUMERIC_LITERAL;
        }
        // (DecimalLiteral)
        @yyt1 ("0" | [1-9] [0-9]* ("." [0-9]+)? | "." [0-9]+) {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return NUMERIC_LITERAL;
        }

        // 字符串字面量 (SingleQuotedString)
        @yyt1 "'" ([^'\\\r\n] | "\\".)* "'" {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return STRING_LITERAL;
        }
        // (DoubleQuotedString)
        @yyt1 '"' ([^"\\\r\n] | "\\".)* '"' {
            YYACTION_RESET_ASI;
            yylval->str_val = strndup((const char*)yyt1, scanner->cursor - yyt1);
            return STRING_LITERAL;
        }

        // --- EOF 和错误处理 ---
        $ { 
            return 0; // Bison 的 EOF 信号
        }

        * {
            // 匹配所有其他字符
            YYACTION_RESET_ASI;
            fprintf(stderr, "Lexical error: Unexpected character '%.*s' at line %d\n", 
                (int)(scanner->cursor - scanner->cursor + 1), scanner->cursor, scanner->line);
            goto yyc_start; // 跳过非法字符
        }
    */
}