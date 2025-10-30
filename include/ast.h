#ifndef AST_H
#define AST_H

#include "common.h"

// AST 节点类型
typedef enum {
    NODE_SCRIPT,              // 顶层脚本 (语句列表的容器)
    NODE_BLOCK_STATEMENT,     // { ... }
    NODE_STATEMENT_LIST,      // (文章中 create_statement_list 暗示的类型，我将其合并到 NODE_SCRIPT)
    NODE_VARIABLE_DECLARATION,// let, const, var
    NODE_VARIABLE_DECLARATOR, // foo, bar = 1
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_THIS_EXPRESSION,
    NODE_IF_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_FUNCTION_DECLARATION,
    NODE_BINARY_EXPRESSION,
    NODE_ASSIGNMENT_EXPRESSION,
    NODE_UNARY_EXPRESSION,
    NODE_CALL_EXPRESSION,
    NODE_MEMBER_EXPRESSION,
    NODE_ARGUMENT_LIST,       // (参数列表的容器)
} NodeType;

// 变量声明类型
typedef enum {
    DECL_LET,
    DECL_CONST,
    DECL_VAR
} DeclarationType;

// 字面量类型
typedef enum {
    LITERAL_NUMBER,
    LITERAL_STRING,
    LITERAL_TRUE,
    LITERAL_FALSE,
    LITERAL_NULL
} LiteralType;

// 二元操作符
typedef enum {
    OP_COMMA, OP_ASSIGN, OP_LOGICAL_OR, OP_LOGICAL_AND,
    OP_BIT_OR, OP_BIT_XOR, OP_BIT_AND,
    OP_EQ, OP_NE, OP_STRICT_EQ, OP_STRICT_NE,
    OP_LT, OP_LE, OP_GT, OP_GE, OP_IN, OP_INSTANCEOF,
    OP_LSHIFT, OP_RSHIFT, OP_URSHIFT,
    OP_PLUS, OP_MINUS,
    OP_MUL, OP_MOD, OP_POWER
    // ... 可根据需要添加其他赋值操作符 ...
} BinaryOpType;

// 一元操作符
typedef enum {
    OP_DELETE, OP_VOID, OP_TYPEOF,
    OP_INC, OP_DEC, // (前缀)
    OP_POST_INC, OP_POST_DEC, // (后缀)
    OP_NOT, OP_BIT_NOT,
    OP_UNARY_PLUS, OP_UNARY_MINUS
} UnaryOpType;

// 抽象语法树 (AST) 节点
typedef struct ASTNode {
    NodeType type;
    int line;                // 节点所在的行号
    struct ASTNode *next;    // 用于连接列表中的节点 (如语句、声明)

    union {
        // NODE_SCRIPT, NODE_BLOCK_STATEMENT
        struct {
            struct ASTNode *head; // 指向语句列表的第一个节点
        } script;

        // NODE_VARIABLE_DECLARATION
        struct {
            DeclarationType decl_type;
            struct ASTNode *head; // 指向声明符列表的第一个节点
        } var_decl;

        // NODE_VARIABLE_DECLARATOR
        struct {
            struct ASTNode *id;   // IDENTIFIER
            struct ASTNode *init; // (可选) assignment_expression
        } var_declarator;

        // NODE_IDENTIFIER
        struct {
            char *name;
        } identifier;

        // NODE_LITERAL
        struct {
            LiteralType type;
            char *value;
        } literal;

        // NODE_IF_STATEMENT
        struct {
            struct ASTNode *test;
            struct ASTNode *consequent;
            struct ASTNode *alternate; // (可选) else
        } if_stmt;

        // NODE_EXPRESSION_STATEMENT
        struct {
            struct ASTNode *expression;
        } expr_stmt;

        // NODE_RETURN_STATEMENT
        struct {
            struct ASTNode *argument; // (可选)
        } return_stmt;

        // NODE_FUNCTION_DECLARATION
        struct {
            struct ASTNode *id;     // Identifier (函数名)
            struct ASTNode *params; // NODE_ARGUMENT_LIST (参数列表)
            struct ASTNode *body;   // NODE_BLOCK_STATEMENT (函数体)
        } func_decl;

        // NODE_BINARY_EXPRESSION
        struct {
            BinaryOpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_expr;

        // NODE_ASSIGNMENT_EXPRESSION
        struct {
            BinaryOpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } assignment_expr;

        // NODE_UNARY_EXPRESSION
        struct {
            UnaryOpType op;
            struct ASTNode *argument;
            bool prefix; // 区分前缀(true)还是后缀(false)
        } unary_expr;

        // NODE_CALL_EXPRESSION
        struct {
            struct ASTNode *callee;
            struct ASTNode *arguments; // 指向 NODE_ARGUMENT_LIST
        } call_expr;

        // NODE_MEMBER_EXPRESSION
        struct {
            struct ASTNode *object;
            struct ASTNode *property;
            bool computed; // true: a[b], false: a.b
        } member_expr;

        // NODE_ARGUMENT_LIST
        struct {
            struct ASTNode *head; // 指向参数列表的第一个节点
        } arg_list;
        
        // NODE_THIS_EXPRESSION (无特定字段)
    } data;
} ASTNode;

// --- AST 辅助函数声明 ---

// 创建节点
ASTNode* create_statement_list(ASTNode *statement, ASTNode *ignored);
ASTNode* create_block_statement(ASTNode *statement_list_head);
ASTNode* create_declaration_list(DeclarationType type, ASTNode *declarator, ASTNode *ignored);
ASTNode* create_variable_declarator(ASTNode *id, ASTNode *init);
ASTNode* create_identifier_node(char *name);
ASTNode* create_literal_node(LiteralType type, char *value);
ASTNode* create_this_node();
ASTNode* create_if_statement(ASTNode *test, ASTNode *consequent, ASTNode *alternate);
ASTNode* create_expression_statement(ASTNode *expression);
ASTNode* create_return_statement(ASTNode *argument);
ASTNode* create_function_declaration(ASTNode *id, ASTNode *params, ASTNode *body);
ASTNode* create_binary_expr(BinaryOpType op, ASTNode *left, ASTNode *right);
ASTNode* create_assignment_expr(BinaryOpType op, ASTNode *left, ASTNode *right);
ASTNode* create_unary_expr(UnaryOpType op, ASTNode *argument, bool prefix); // 扩展了 API
ASTNode* create_call_expression(ASTNode *callee, ASTNode *arguments);
ASTNode* create_member_access(ASTNode *object, ASTNode *property, bool computed);
ASTNode* create_argument_list(ASTNode *argument, ASTNode *ignored);

// 列表操作
ASTNode* append_to_list(ASTNode *list_wrapper, ASTNode *node_to_append);

// ASI 辅助函数
bool can_insert_semicolon(Scanner *scanner);

// 内存管理
void free_ast(ASTNode *node);

// 调试
void print_ast(ASTNode *node, int indent);
#endif // AST_H