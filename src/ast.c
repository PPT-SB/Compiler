#include "ast.h"
#include <stdlib.h>
#include <string.h>

//列表创建函数
NodeList* nodelist_create(void) {
    NodeList *list = (NodeList*)malloc(sizeof(NodeList));
    if (!list) {
        yyerror("Out of memory");
        exit(1);
    }
    list->size = 0;
    list->capacity = 4; // 初始容量为 4
    list->nodes = (ASTNode**)malloc(list->capacity * sizeof(ASTNode*));
    if (!list->nodes) {
        yyerror("Out of memory");
        exit(1);
    }
    return list;
}
// 内部辅助函数：创建一个基础节点
static ASTNode* create_base_node(NodeType type) {
    extern Scanner* scanner; // 访问在 main.c 中定义的全局 scanner
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        yyerror("Out of memory");
        exit(1);
    }
    node->type = type;
    node->line = scanner ? scanner->line : -1; // 尽可能记录行号
    node->next = NULL;
    return node;
}

// --- 节点创建函数 ---

ASTNode* create_statement_list(ASTNode *statement, ASTNode *ignored) {
    ASTNode *list = create_base_node(NODE_SCRIPT);
    list->data.script.head = statement;
    if (statement) {
        statement->next = NULL;
    }
    return list;
}

ASTNode* create_block_statement(ASTNode *statement_list_node) {
    ASTNode *block = create_base_node(NODE_BLOCK_STATEMENT);

    if (statement_list_node && statement_list_node->type == NODE_SCRIPT) {
        // 拆包：拿走语句链表的头节点
        block->data.script.head = statement_list_node->data.script.head;
        // 释放掉多余的 NODE_SCRIPT 包装器
        free(statement_list_node); 
    } else {
        // 如果是空块 ({} LBRACE RBRACE)，$2 是 NULL
        block->data.script.head = NULL;
    }
    return block;
}

ASTNode* create_declaration_list(DeclarationType type, ASTNode *declarator, ASTNode *ignored) {
    ASTNode *list = create_base_node(NODE_VARIABLE_DECLARATION);
    list->data.var_decl.decl_type = type;
    list->data.var_decl.head = declarator;
    if (declarator) {
        declarator->next = NULL;
    }
    return list;
}

ASTNode* create_variable_declarator(ASTNode *id, ASTNode *init) {
    ASTNode *node = create_base_node(NODE_VARIABLE_DECLARATOR);
    node->data.var_declarator.id = id;
    node->data.var_declarator.init = init;
    return node;
}

ASTNode* create_identifier_node(char *name) {
    ASTNode *node = create_base_node(NODE_IDENTIFIER);
    node->data.identifier.name = name; // strndup 已经在 lexer.re 中完成
    return node;
}

ASTNode* create_literal_node(LiteralType type, char *value) {
    ASTNode *node = create_base_node(NODE_LITERAL);
    node->data.literal.type = type;
    node->data.literal.value = value;
    return node;
}

ASTNode* create_this_node() {
    return create_base_node(NODE_THIS_EXPRESSION);
}

ASTNode* create_if_statement(ASTNode *test, ASTNode *consequent, ASTNode *alternate) {
    ASTNode *node = create_base_node(NODE_IF_STATEMENT);
    node->data.if_stmt.test = test;
    node->data.if_stmt.consequent = consequent;
    node->data.if_stmt.alternate = alternate;
    return node;
}

ASTNode* create_expression_statement(ASTNode *expression) {
    ASTNode *node = create_base_node(NODE_EXPRESSION_STATEMENT);
    node->data.expr_stmt.expression = expression;
    return node;
}

ASTNode* create_return_statement(ASTNode *argument) {
    ASTNode *node = create_base_node(NODE_RETURN_STATEMENT);
    node->data.return_stmt.argument = argument;
    return node;
}

ASTNode* create_function_declaration(ASTNode *id, NodeList *params, ASTNode *body) {
    ASTNode *node = create_base_node(NODE_FUNCTION_DECLARATION);
    node->data.func_decl.id = id;
    node->data.func_decl.params = params;
    node->data.func_decl.body = body;
    return node;
}

ASTNode* create_binary_expr(BinaryOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_base_node(NODE_BINARY_EXPRESSION);
    node->data.binary_expr.op = op;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    return node;
}

ASTNode* create_assignment_expr(BinaryOpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = create_base_node(NODE_ASSIGNMENT_EXPRESSION);
    node->data.assignment_expr.op = op;
    node->data.assignment_expr.left = left;
    node->data.assignment_expr.right = right;
    return node;
}

// 注意：我扩展了此 API 以处理前缀/后缀
ASTNode* create_unary_expr(UnaryOpType op, ASTNode *argument, bool prefix) {
    ASTNode *node = create_base_node(NODE_UNARY_EXPRESSION);
    node->data.unary_expr.op = op;
    node->data.unary_expr.argument = argument;
    node->data.unary_expr.prefix = prefix;
    return node;
}

ASTNode* create_call_expression(ASTNode *callee, NodeList *arguments) {
    ASTNode *node = create_base_node(NODE_CALL_EXPRESSION);
    node->data.call_expr.callee = callee;
    node->data.call_expr.arguments = arguments; // 这是一个 NODE_ARGUMENT_LIST
    return node;
}

ASTNode* create_member_access(ASTNode *object, ASTNode *property, bool computed) {
    ASTNode *node = create_base_node(NODE_MEMBER_EXPRESSION);
    node->data.member_expr.object = object;
    node->data.member_expr.property = property;
    node->data.member_expr.computed = computed;
    return node;
}

// 列表操作
void nodelist_append(NodeList* list, ASTNode* node) {
    if (list->size >= list->capacity) {
        // 数组已满，容量加倍
        list->capacity *= 2;
        list->nodes = (ASTNode**)realloc(list->nodes, list->capacity * sizeof(ASTNode*));
        if (!list->nodes) {
            yyerror("Out of memory");
            exit(1);
        }
    }
    list->nodes[list->size++] = node;
}

// 列表操作 (仍被 statement_list 和 variable_declaration_list 使用)
ASTNode* append_to_list(ASTNode *list_wrapper, ASTNode *node_to_append) {
    if (!list_wrapper || !node_to_append) return list_wrapper;

    ASTNode *head = NULL;

    // 根据包装器类型找到 head 指针
    if (list_wrapper->type == NODE_SCRIPT || list_wrapper->type == NODE_BLOCK_STATEMENT) {
        head = list_wrapper->data.script.head;
    } else if (list_wrapper->type == NODE_VARIABLE_DECLARATION) {
        head = list_wrapper->data.var_decl.head;
    } else {
        // 错误：试图向非列表类型追加
        return list_wrapper;
    }

    if (head == NULL) {
        // 列表为空，设置 head
        if (list_wrapper->type == NODE_SCRIPT || list_wrapper->type == NODE_BLOCK_STATEMENT) {
            list_wrapper->data.script.head = node_to_append;
        } else if (list_wrapper->type == NODE_VARIABLE_DECLARATION) {
            list_wrapper->data.var_decl.head = node_to_append;
        }
    } else {
        // 列表不为空，找到尾部并追加
        ASTNode *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node_to_append;
    }

    node_to_append->next = NULL;
    return list_wrapper;
}

// ASI 辅助函数 (来自 4.3 节)
void nodelist_free(NodeList* list) {
    if (!list) return;
    for (int i = 0; i < list->size; i++) {
        // 递归释放列表中的每个节点
        free_ast(list->nodes[i]);
    }
    // 释放指针数组本身
    free(list->nodes);
    // 释放列表结构体
    free(list);
}

bool can_insert_semicolon(Scanner *scanner) {
    // 简化的实现：如果看到了换行符，就允许插入
    // 完整的实现还需要检查下一个 token 是否是 '}' 或 EOF
    // (这需要Bison的 lookahead 功能，比较复杂)
    return scanner->line_terminator_seen;
}

// 内存管理
void free_ast(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_SCRIPT:
        case NODE_BLOCK_STATEMENT:
            free_ast(node->data.script.head);
            break;
        case NODE_VARIABLE_DECLARATION:
            free_ast(node->data.var_decl.head);
            break;
        case NODE_VARIABLE_DECLARATOR:
            free_ast(node->data.var_declarator.id);
            free_ast(node->data.var_declarator.init);
            break;
        case NODE_IDENTIFIER:
            free(node->data.identifier.name);
            break;
        case NODE_LITERAL:
            free(node->data.literal.value);
            break;
        case NODE_IF_STATEMENT:
            free_ast(node->data.if_stmt.test);
            free_ast(node->data.if_stmt.consequent);
            free_ast(node->data.if_stmt.alternate);
            break;
        case NODE_EXPRESSION_STATEMENT:
            free_ast(node->data.expr_stmt.expression);
            break;
        case NODE_RETURN_STATEMENT:
            free_ast(node->data.return_stmt.argument);
            break;
        case NODE_FUNCTION_DECLARATION:
            free_ast(node->data.func_decl.id);
            nodelist_free(node->data.func_decl.params);
            free_ast(node->data.func_decl.body);
        break;
        case NODE_BINARY_EXPRESSION:
            free_ast(node->data.binary_expr.left);
            free_ast(node->data.binary_expr.right);
            break;
        case NODE_ASSIGNMENT_EXPRESSION:
            free_ast(node->data.assignment_expr.left);
            free_ast(node->data.assignment_expr.right);
            break;
        case NODE_UNARY_EXPRESSION:
            free_ast(node->data.unary_expr.argument);
            break;
        case NODE_CALL_EXPRESSION:
            free_ast(node->data.call_expr.callee);
            nodelist_free(node->data.call_expr.arguments);
            break;
        case NODE_MEMBER_EXPRESSION:
            free_ast(node->data.member_expr.object);
            free_ast(node->data.member_expr.property);
            break;
        case NODE_THIS_EXPRESSION:
            break;
        default:
            // 未知节点类型
            break;
    }

    // 递归释放兄弟节点
    free_ast(node->next);
    // 最后释放自己
    free(node);
}

// --- 调试：打印 AST ---
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

static void nodelist_print(NodeList* list, int indent) {
    if (!list || list->size == 0) {
        print_indent(indent);
        printf("[] (empty)\n"); // ESTree uses [] for arrays
        return;
    }

    print_indent(indent);
    printf("[\n");
    for (int i = 0; i < list->size; i++) {
        print_ast(list->nodes[i], indent + 1);
    }
    print_indent(indent);
    printf("]\n");
}

static const char* literal_type_to_str(LiteralType type) {
    switch (type) {
        case LITERAL_NUMBER: return "Number";
        case LITERAL_STRING: return "String";
        case LITERAL_TRUE:   return "Boolean";
        case LITERAL_FALSE:  return "Boolean";
        case LITERAL_NULL:   return "Null";
        default: return "Unknown";
    }
}

static const char* bin_op_to_str(BinaryOpType op) {
    switch (op) {
        case OP_PLUS: return "+";
        case OP_MINUS: return "-";
        case OP_MUL: return "*";
        case OP_COMMA: return ",";
        case OP_ASSIGN: return "=";
        case OP_EQ: return "==";
        case OP_STRICT_EQ: return "===";
        case OP_NE: return "!=";          
        case OP_STRICT_NE: return "!=="; 
        case OP_LT: return "<";
        case OP_LE: return "<=";         
        case OP_GT: return ">";           
        case OP_GE: return ">=";
        default: return "?_OP_?";
    }
}

static const char* un_op_to_str(UnaryOpType op) {
    switch (op) {
        case OP_NOT: return "!";
        case OP_DELETE: return "delete";
        case OP_TYPEOF: return "typeof";
        // ...
        default: return "?_UOP_?";
    }
}

void print_ast(ASTNode *node, int indent) {
    if (!node) {
        print_indent(indent);
        printf("(null)\n");
        return;
    }

    print_indent(indent);

    switch (node->type) {
        case NODE_SCRIPT:
            printf("Script\n");
            print_ast(node->data.script.head, indent + 1);
            break;
        case NODE_BLOCK_STATEMENT:
            printf("BlockStatement\n");
            print_ast(node->data.script.head, indent + 1);
            break;
        case NODE_VARIABLE_DECLARATION:
            printf("VariableDeclaration (%s)\n", node->data.var_decl.decl_type == DECL_LET ? "let" : "const");
            print_ast(node->data.var_decl.head, indent + 1);
            break;
        case NODE_VARIABLE_DECLARATOR:
            printf("VariableDeclarator\n");
            print_indent(indent + 1); printf("id:\n");
            print_ast(node->data.var_declarator.id, indent + 2);
            print_indent(indent + 1); printf("init:\n");
            print_ast(node->data.var_declarator.init, indent + 2);
            break;
        case NODE_IDENTIFIER:
            printf("Identifier (name: %s)\n", node->data.identifier.name);
            break;
        case NODE_LITERAL:
            printf("Literal (type: %s, value: %s)\n", 
                   literal_type_to_str(node->data.literal.type), 
                   node->data.literal.value);
            break;
        case NODE_THIS_EXPRESSION:
            printf("ThisExpression\n");
            break;
        case NODE_IF_STATEMENT:
            printf("IfStatement\n");
            print_indent(indent + 1); printf("test:\n");
            print_ast(node->data.if_stmt.test, indent + 2);
            print_indent(indent + 1); printf("consequent:\n");
            print_ast(node->data.if_stmt.consequent, indent + 2);
            print_indent(indent + 1); printf("alternate:\n");
            print_ast(node->data.if_stmt.alternate, indent + 2);
            break;
        case NODE_EXPRESSION_STATEMENT:
            printf("ExpressionStatement\n");
            print_ast(node->data.expr_stmt.expression, indent + 1);
            break;
        case NODE_RETURN_STATEMENT:
            printf("ReturnStatement\n");
            print_indent(indent + 1); printf("argument:\n");
            print_ast(node->data.return_stmt.argument, indent + 2);
            break;
        case NODE_FUNCTION_DECLARATION:
            printf("FunctionDeclaration\n");
            print_indent(indent + 1); printf("id:\n");
            print_ast(node->data.func_decl.id, indent + 2);
            print_indent(indent + 1); printf("params:\n");
            nodelist_print(node->data.func_decl.params, indent + 2);
            print_indent(indent + 1); printf("body:\n");
            print_ast(node->data.func_decl.body, indent + 2);
            break;
        case NODE_BINARY_EXPRESSION:
            printf("BinaryExpression (op: %s)\n", bin_op_to_str(node->data.binary_expr.op));
            print_indent(indent + 1); printf("left:\n");
            print_ast(node->data.binary_expr.left, indent + 2);
            print_indent(indent + 1); printf("right:\n");
            print_ast(node->data.binary_expr.right, indent + 2);
            break;
        case NODE_ASSIGNMENT_EXPRESSION:
            printf("AssignmentExpression (op: %s)\n", bin_op_to_str(node->data.assignment_expr.op));
            print_indent(indent + 1); printf("left:\n");
            print_ast(node->data.assignment_expr.left, indent + 2);
            print_indent(indent + 1); printf("right:\n");
            print_ast(node->data.assignment_expr.right, indent + 2);
            break;
        case NODE_UNARY_EXPRESSION:
            printf("UnaryExpression (op: %s, prefix: %d)\n", un_op_to_str(node->data.unary_expr.op), node->data.unary_expr.prefix);
            print_ast(node->data.unary_expr.argument, indent + 1);
            break;
        case NODE_CALL_EXPRESSION:
            printf("CallExpression\n");
            print_indent(indent + 1); printf("callee:\n");
            print_ast(node->data.call_expr.callee, indent + 2);
            print_indent(indent + 1); printf("arguments:\n");
            nodelist_print(node->data.call_expr.arguments, indent + 2);
            break;
        case NODE_MEMBER_EXPRESSION:
            printf("MemberExpression (computed: %d)\n", node->data.member_expr.computed);
            print_indent(indent + 1); printf("object:\n");
            print_ast(node->data.member_expr.object, indent + 2);
            print_indent(indent + 1); printf("property:\n");
            print_ast(node->data.member_expr.property, indent + 2);
            break;
        default:
            printf("UnknownNode (type: %d)\n", node->type);
    }
    
    // 打印兄弟节点
    if (node->next) {
        print_ast(node->next, indent);
    }
}