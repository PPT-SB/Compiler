#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "ast.h"

// 定义 parser.y 中声明的全局变量
Scanner *scanner;
ASTNode *ast_root;

// yyparse() 是由 Bison 生成的
extern int yyparse();

// main 函数
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename.js>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Error opening file");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char*)malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate memory for file buffer\n");
        fclose(fp);
        return 1;
    }

    if (fread(buffer, 1, size, fp) != size) {
        fprintf(stderr, "Failed to read file\n");
        fclose(fp);
        free(buffer);
        return 1;
    }
    buffer[size] = 0; // NULL 终止，re2c 的 $ 规则需要
    fclose(fp);

    // 初始化 Scanner
    Scanner s;
    s.cursor = buffer;
    s.limit = buffer + size;
    s.marker = buffer; // re2c 需要 marker
    s.line = 1;
    s.line_terminator_seen = false;
    s.restrict_new_line = false;
    
    // 设置全局扫描器指针
    scanner = &s;
    ast_root = NULL;

    // 调用解析器
    int result = yyparse();

    if (result == 0) {
        printf("Parse successful!\n");
        printf("--- AST ---\n");
        print_ast(ast_root, 0);
        printf("-----------\n");
    } else {
        printf("Parse failed.\n");
    }

    // 清理
    free_ast(ast_root);
    free(buffer);

    return result;
}

// 错误报告函数 (parser.y 需要)
void yyerror(const char *s){
    fprintf(stderr, "Parse error: %s at line %d\n", s, scanner ? scanner->line : -1);
}