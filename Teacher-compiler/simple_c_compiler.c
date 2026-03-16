/*
 * Simple C Compiler - 一个简单的C语言编译器
 * 
 * 功能:
 * 1. 词法分析 (Lexer) - 将源代码转换为Token序列
 * 2. 语法分析 (Parser) - 构建抽象语法树(AST)
 * 3. 语义分析 (Semantic) - 类型检查和符号表管理
 * 4. 代码生成 (Code Generator) - 生成x86汇编代码
 * 
 * 支持的语法:
 * - 变量声明: int, float, char
 * - 算术运算: +, -, *, /, %
 * - 比较运算: ==, !=, <, >, <=, >=
 * - 逻辑运算: &&, ||, !
 * - 控制语句: if-else, while, for
 * - 函数定义和调用
 * - return语句
 * 
 * 作者: Qingyan Agent
 * 日期: 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* ==================== 宏定义 ==================== */

#define MAX_SOURCE_SIZE 65536      // 源代码最大长度
#define MAX_TOKEN_LENGTH 256       // Token最大长度
#define MAX_SYMBOLS 1024           // 符号表最大容量
#define MAX_SCOPES 100             // 最大作用域嵌套层数
#define MAX_AST_NODES 10000        // AST节点最大数量
#define MAX_CODE_LENGTH 65536      // 生成的代码最大长度
#define MAX_PARAMS 10              // 函数最大参数数量
#define MAX_LOCAL_VARS 100         // 最大局部变量数量

/* ==================== Token类型定义 ==================== */

typedef enum {
    // 关键字
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_VOID,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    
    // 标识符和字面量
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOAT_NUM,
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,
    
    // 运算符
    TOKEN_PLUS,          // +
    TOKEN_MINUS,         // -
    TOKEN_STAR,          // *
    TOKEN_SLASH,         // /
    TOKEN_PERCENT,       // %
    TOKEN_ASSIGN,        // =
    TOKEN_EQ,            // ==
    TOKEN_NE,            // !=
    TOKEN_LT,            // <
    TOKEN_GT,            // >
    TOKEN_LE,            // <=
    TOKEN_GE,            // >=
    TOKEN_AND,           // &&
    TOKEN_OR,            // ||
    TOKEN_NOT,           // !
    TOKEN_AMPERSAND,     // &
    
    // 分隔符
    TOKEN_LPAREN,        // (
    TOKEN_RPAREN,        // )
    TOKEN_LBRACE,        // {
    TOKEN_RBRACE,        // }
    TOKEN_LBRACKET,      // [
    TOKEN_RBRACKET,      // ]
    TOKEN_SEMICOLON,     // ;
    TOKEN_COMMA,         // ,
    
    // 特殊Token
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

/* ==================== Token结构体 ==================== */

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
    int line;
    int column;
} Token;

/* ==================== 符号表相关 ==================== */

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_UNKNOWN
} DataType;

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolKind;

typedef struct Symbol {
    char name[MAX_TOKEN_LENGTH];
    DataType type;
    SymbolKind kind;
    int scope_level;
    int offset;              // 栈偏移量
    int param_count;         // 函数参数数量(仅用于函数)
    DataType param_types[MAX_PARAMS];  // 参数类型
    bool is_defined;         // 是否已定义
} Symbol;

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int current_scope;
    int stack_offset[MAX_SCOPES];
} SymbolTable;

/* ==================== AST节点类型 ==================== */

typedef enum {
    // 程序结构
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    
    // 声明
    NODE_VAR_DECL,
    NODE_PARAM,
    
    // 语句
    NODE_COMPOUND_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_EXPR_STMT,
    NODE_BREAK_STMT,
    NODE_CONTINUE_STMT,
    
    // 表达式
    NODE_ASSIGN_EXPR,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_ARRAY_ACCESS,
    NODE_IDENTIFIER,
    NODE_INTEGER,
    NODE_FLOAT_NUM,
    NODE_CHAR_LITERAL,
    NODE_STRING_LITERAL
} NodeType;

/* ==================== AST节点结构体 ==================== */

typedef struct ASTNode {
    NodeType type;
    int line;
    
    // 用于标识符和字面量
    char name[MAX_TOKEN_LENGTH];
    DataType data_type;
    int int_value;
    float float_value;
    char char_value;
    
    // 子节点
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* condition;
    struct ASTNode* then_branch;
    struct ASTNode* else_branch;
    struct ASTNode* body;
    struct ASTNode* init;
    struct ASTNode* update;
    struct ASTNode* args[MAX_PARAMS];
    int arg_count;
    
    // 用于函数声明
    struct ASTNode* params[MAX_PARAMS];
    int param_count;
    struct ASTNode* return_type;
    
    // 用于变量声明
    struct ASTNode* var_type;
    struct ASTNode* initializer;
    
    // 用于数组访问
    struct ASTNode* index;
    
    // 用于复合语句
    struct ASTNode* statements[100];
    int stmt_count;
    
    // 代码生成用
    int offset;
} ASTNode;

/* ==================== 全局变量 ==================== */

char source_code[MAX_SOURCE_SIZE];
int source_pos = 0;
int source_length = 0;
int current_line = 1;
int current_column = 1;

Token current_token;
Token next_token;

SymbolTable symbol_table;
ASTNode* ast_nodes[MAX_AST_NODES];
int ast_node_count = 0;

char generated_code[MAX_CODE_LENGTH];
int code_pos = 0;

int label_counter = 0;
int temp_var_counter = 0;

/* ==================== 关键字表 ==================== */

typedef struct {
    const char* name;
    TokenType type;
} Keyword;

Keyword keywords[] = {
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT},
    {"char", TOKEN_CHAR},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {NULL, TOKEN_ERROR}
};

/* ==================== 辅助函数 ==================== */

// 创建新的AST节点
ASTNode* create_node(NodeType type) {
    if (ast_node_count >= MAX_AST_NODES) {
        fprintf(stderr, "Error: Too many AST nodes\n");
        exit(1);
    }
    
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->line = current_line;
    ast_nodes[ast_node_count++] = node;
    
    return node;
}

// 获取数据类型名称
const char* get_type_name(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR: return "char";
        case TYPE_VOID: return "void";
        default: return "unknown";
    }
}

// 获取Token类型名称
const char* get_token_name(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "int";
        case TOKEN_FLOAT: return "float";
        case TOKEN_CHAR: return "char";
        case TOKEN_VOID: return "void";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FOR: return "for";
        case TOKEN_RETURN: return "return";
        case TOKEN_BREAK: return "break";
        case TOKEN_CONTINUE: return "continue";
        case TOKEN_IDENTIFIER: return "identifier";
        case TOKEN_INTEGER: return "integer";
        case TOKEN_FLOAT_NUM: return "float";
        case TOKEN_CHAR_LITERAL: return "char_literal";
        case TOKEN_STRING_LITERAL: return "string";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_PERCENT: return "%";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_EQ: return "==";
        case TOKEN_NE: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_GT: return ">";
        case TOKEN_LE: return "<=";
        case TOKEN_GE: return ">=";
        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        case TOKEN_NOT: return "!";
        case TOKEN_AMPERSAND: return "&";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COMMA: return ",";
        case TOKEN_EOF: return "EOF";
        default: return "unknown";
    }
}

/* ==================== 词法分析器 (Lexer) ==================== */

// 读取源文件
bool load_source(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return false;
    }
    
    source_length = fread(source_code, 1, MAX_SOURCE_SIZE - 1, file);
    source_code[source_length] = '\0';
    fclose(file);
    
    source_pos = 0;
    current_line = 1;
    current_column = 1;
    
    return true;
}

// 查看当前字符
char peek_char() {
    if (source_pos >= source_length) return '\0';
    return source_code[source_pos];
}

// 查看下一个字符
char peek_next_char() {
    if (source_pos + 1 >= source_length) return '\0';
    return source_code[source_pos + 1];
}

// 读取下一个字符
char read_char() {
    if (source_pos >= source_length) return '\0';
    
    char c = source_code[source_pos++];
    
    if (c == '\n') {
        current_line++;
        current_column = 1;
    } else {
        current_column++;
    }
    
    return c;
}

// 跳过空白字符
void skip_whitespace() {
    while (source_pos < source_length) {
        char c = peek_char();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            read_char();
        } else if (c == '/' && peek_next_char() == '/') {
            // 单行注释
            while (source_pos < source_length && peek_char() != '\n') {
                read_char();
            }
        } else if (c == '/' && peek_next_char() == '*') {
            // 多行注释
            read_char(); // /
            read_char(); // *
            while (source_pos < source_length) {
                if (peek_char() == '*' && peek_next_char() == '/') {
                    read_char();
                    read_char();
                    break;
                }
                read_char();
            }
        } else {
            break;
        }
    }
}

// 检查是否为关键字
TokenType check_keyword(const char* name) {
    for (int i = 0; keywords[i].name != NULL; i++) {
        if (strcmp(name, keywords[i].name) == 0) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

// 读取标识符或关键字
Token read_identifier() {
    Token token;
    token.line = current_line;
    token.column = current_column;
    
    int i = 0;
    while (source_pos < source_length) {
        char c = peek_char();
        if (isalnum(c) || c == '_') {
            token.value[i++] = read_char();
        } else {
            break;
        }
    }
    token.value[i] = '\0';
    token.type = check_keyword(token.value);
    
    return token;
}

// 读取数字
Token read_number() {
    Token token;
    token.line = current_line;
    token.column = current_column;
    token.type = TOKEN_INTEGER;
    
    int i = 0;
    bool has_dot = false;
    
    while (source_pos < source_length) {
        char c = peek_char();
        if (isdigit(c)) {
            token.value[i++] = read_char();
        } else if (c == '.' && !has_dot) {
            has_dot = true;
            token.type = TOKEN_FLOAT_NUM;
            token.value[i++] = read_char();
        } else {
            break;
        }
    }
    token.value[i] = '\0';
    
    return token;
}

// 读取字符字面量
Token read_char_literal() {
    Token token;
    token.line = current_line;
    token.column = current_column;
    token.type = TOKEN_CHAR_LITERAL;
    
    read_char(); // 读取开头的单引号
    
    char c = read_char();
    if (c == '\\') {
        // 转义字符
        c = read_char();
        switch (c) {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case '0': c = '\0'; break;
            case '\\': c = '\\'; break;
            case '\'': c = '\''; break;
            default: break;
        }
    }
    token.char_value = c;
    token.value[0] = c;
    token.value[1] = '\0';
    
    if (peek_char() == '\'') {
        read_char(); // 读取结尾的单引号
    }
    
    return token;
}

// 读取字符串字面量
Token read_string_literal() {
    Token token;
    token.line = current_line;
    token.column = current_column;
    token.type = TOKEN_STRING_LITERAL;
    
    read_char(); // 读取开头的双引号
    
    int i = 0;
    while (source_pos < source_length && peek_char() != '"') {
        char c = read_char();
        if (c == '\\') {
            c = read_char();
            switch (c) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '0': c = '\0'; break;
                case '\\': c = '\\'; break;
                case '"': c = '"'; break;
                default: break;
            }
        }
        token.value[i++] = c;
    }
    token.value[i] = '\0';
    
    if (peek_char() == '"') {
        read_char(); // 读取结尾的双引号
    }
    
    return token;
}

// 获取下一个Token
Token get_next_token() {
    skip_whitespace();
    
    Token token;
    token.line = current_line;
    token.column = current_column;
    
    if (source_pos >= source_length) {
        token.type = TOKEN_EOF;
        strcpy(token.value, "EOF");
        return token;
    }
    
    char c = peek_char();
    
    // 标识符或关键字
    if (isalpha(c) || c == '_') {
        return read_identifier();
    }
    
    // 数字
    if (isdigit(c)) {
        return read_number();
    }
    
    // 字符字面量
    if (c == '\'') {
        return read_char_literal();
    }
    
    // 字符串字面量
    if (c == '"') {
        return read_string_literal();
    }
    
    // 运算符和分隔符
    read_char();
    token.value[0] = c;
    token.value[1] = '\0';
    
    switch (c) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '%': token.type = TOKEN_PERCENT; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '&': token.type = TOKEN_AMPERSAND; break;
        case '!':
            if (peek_char() == '=') {
                read_char();
                token.type = TOKEN_NE;
                strcpy(token.value, "!=");
            } else {
                token.type = TOKEN_NOT;
            }
            break;
        case '=':
            if (peek_char() == '=') {
                read_char();
                token.type = TOKEN_EQ;
                strcpy(token.value, "==");
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '<':
            if (peek_char() == '=') {
                read_char();
                token.type = TOKEN_LE;
                strcpy(token.value, "<=");
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (peek_char() == '=') {
                read_char();
                token.type = TOKEN_GE;
                strcpy(token.value, ">=");
            } else {
                token.type = TOKEN_GT;
            }
            break;
        case '&':
            if (peek_char() == '&') {
                read_char();
                token.type = TOKEN_AND;
                strcpy(token.value, "&&");
            } else {
                token.type = TOKEN_AMPERSAND;
            }
            break;
        case '|':
            if (peek_char() == '|') {
                read_char();
                token.type = TOKEN_OR;
                strcpy(token.value, "||");
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
        default:
            token.type = TOKEN_ERROR;
            break;
    }
    
    return token;
}

// 初始化词法分析器
void init_lexer() {
    current_token = get_next_token();
    next_token = get_next_token();
}

// 前进到下一个Token
void advance_token() {
    current_token = next_token;
    next_token = get_next_token();
}

// 匹配并前进
bool match_token(TokenType type) {
    if (current_token.type == type) {
        advance_token();
        return true;
    }
    return false;
}

// 期望特定Token
void expect_token(TokenType type) {
    if (current_token.type != type) {
        fprintf(stderr, "Error at line %d: Expected '%s', got '%s'\n",
                current_token.line, get_token_name(type), get_token_name(current_token.type));
        exit(1);
    }
    advance_token();
}

/* ==================== 符号表管理 ==================== */

// 初始化符号表
void init_symbol_table() {
    symbol_table.count = 0;
    symbol_table.current_scope = 0;
    for (int i = 0; i < MAX_SCOPES; i++) {
        symbol_table.stack_offset[i] = 0;
    }
}

// 进入新作用域
void enter_scope() {
    symbol_table.current_scope++;
    if (symbol_table.current_scope >= MAX_SCOPES) {
        fprintf(stderr, "Error: Too many nested scopes\n");
        exit(1);
    }
    symbol_table.stack_offset[symbol_table.current_scope] = 
        symbol_table.stack_offset[symbol_table.current_scope - 1];
}

// 离开作用域
void leave_scope() {
    // 移除当前作用域的符号
    int i = symbol_table.count - 1;
    while (i >= 0 && symbol_table.symbols[i].scope_level == symbol_table.current_scope) {
        symbol_table.count--;
        i--;
    }
    
    if (symbol_table.current_scope > 0) {
        symbol_table.current_scope--;
    }
}

// 添加符号
bool add_symbol(const char* name, DataType type, SymbolKind kind) {
    // 检查当前作用域是否已存在同名符号
    for (int i = symbol_table.count - 1; i >= 0; i--) {
        if (symbol_table.symbols[i].scope_level < symbol_table.current_scope) {
            break;
        }
        if (strcmp(symbol_table.symbols[i].name, name) == 0) {
            fprintf(stderr, "Error: Symbol '%s' already defined in current scope\n", name);
            return false;
        }
    }
    
    if (symbol_table.count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Symbol table full\n");
        return false;
    }
    
    Symbol* sym = &symbol_table.symbols[symbol_table.count++];
    strcpy(sym->name, name);
    sym->type = type;
    sym->kind = kind;
    sym->scope_level = symbol_table.current_scope;
    sym->is_defined = true;
    
    // 计算栈偏移
    sym->offset = symbol_table.stack_offset[symbol_table.current_scope];
    symbol_table.stack_offset[symbol_table.current_scope] += 4; // 假设所有类型占4字节
    
    return true;
}

// 查找符号
Symbol* lookup_symbol(const char* name) {
    for (int i = symbol_table.count - 1; i >= 0; i--) {
        if (strcmp(symbol_table.symbols[i].name, name) == 0) {
            return &symbol_table.symbols[i];
        }
    }
    return NULL;
}

// 添加函数符号
bool add_function_symbol(const char* name, DataType return_type, 
                         int param_count, DataType param_types[]) {
    if (symbol_table.count >= MAX_SYMBOLS) {
        fprintf(stderr, "Error: Symbol table full\n");
        return false;
    }
    
    Symbol* sym = &symbol_table.symbols[symbol_table.count++];
    strcpy(sym->name, name);
    sym->type = return_type;
    sym->kind = SYMBOL_FUNCTION;
    sym->scope_level = 0; // 函数总是在全局作用域
    sym->param_count = param_count;
    for (int i = 0; i < param_count; i++) {
        sym->param_types[i] = param_types[i];
    }
    sym->is_defined = true;
    
    return true;
}

/* ==================== 语法分析器 (Parser) ==================== */

// 前向声明
ASTNode* parse_expression();
ASTNode* parse_statement();
ASTNode* parse_declaration();
ASTNode* parse_compound_statement();

// 获取类型对应的DataType
DataType get_data_type(TokenType token_type) {
    switch (token_type) {
        case TOKEN_INT: return TYPE_INT;
        case TOKEN_FLOAT: return TYPE_FLOAT;
        case TOKEN_CHAR: return TYPE_CHAR;
        case TOKEN_VOID: return TYPE_VOID;
        default: return TYPE_UNKNOWN;
    }
}

// 解析基本表达式 (primary expression)
ASTNode* parse_primary_expression() {
    ASTNode* node = NULL;
    
    switch (current_token.type) {
        case TOKEN_INTEGER:
            node = create_node(NODE_INTEGER);
            node->int_value = atoi(current_token.value);
            node->data_type = TYPE_INT;
            advance_token();
            break;
            
        case TOKEN_FLOAT_NUM:
            node = create_node(NODE_FLOAT_NUM);
            node->float_value = atof(current_token.value);
            node->data_type = TYPE_FLOAT;
            advance_token();
            break;
            
        case TOKEN_CHAR_LITERAL:
            node = create_node(NODE_CHAR_LITERAL);
            node->char_value = current_token.value[0];
            node->data_type = TYPE_CHAR;
            advance_token();
            break;
            
        case TOKEN_STRING_LITERAL:
            node = create_node(NODE_STRING_LITERAL);
            strcpy(node->name, current_token.value);
            advance_token();
            break;
            
        case TOKEN_IDENTIFIER:
            node = create_node(NODE_IDENTIFIER);
            strcpy(node->name, current_token.value);
            
            // 查找符号获取类型
            Symbol* sym = lookup_symbol(current_token.value);
            if (sym) {
                node->data_type = sym->type;
                node->offset = sym->offset;
            }
            advance_token();
            
            // 函数调用
            if (current_token.type == TOKEN_LPAREN) {
                ASTNode* call_node = create_node(NODE_CALL_EXPR);
                strcpy(call_node->name, node->name);
                call_node->data_type = node->data_type;
                advance_token(); // (
                
                // 解析参数列表
                if (current_token.type != TOKEN_RPAREN) {
                    call_node->arg_count = 0;
                    do {
                        call_node->args[call_node->arg_count++] = parse_expression();
                        if (current_token.type == TOKEN_COMMA) {
                            advance_token();
                        } else {
                            break;
                        }
                    } while (call_node->arg_count < MAX_PARAMS);
                }
                
                expect_token(TOKEN_RPAREN);
                node = call_node;
            }
            // 数组访问
            else if (current_token.type == TOKEN_LBRACKET) {
                ASTNode* array_node = create_node(NODE_ARRAY_ACCESS);
                array_node->left = node;
                advance_token(); // [
                array_node->index = parse_expression();
                expect_token(TOKEN_RBRACKET);
                node = array_node;
            }
            break;
            
        case TOKEN_LPAREN:
            advance_token();
            node = parse_expression();
            expect_token(TOKEN_RPAREN);
            break;
            
        default:
            fprintf(stderr, "Error at line %d: Unexpected token '%s'\n",
                    current_token.line, get_token_name(current_token.type));
            advance_token();
            break;
    }
    
    return node;
}

// 解析一元表达式
ASTNode* parse_unary_expression() {
    if (current_token.type == TOKEN_MINUS || 
        current_token.type == TOKEN_NOT ||
        current_token.type == TOKEN_AMPERSAND) {
        
        ASTNode* node = create_node(NODE_UNARY_EXPR);
        node->name[0] = current_token.value[0];
        node->name[1] = '\0';
        advance_token();
        node->left = parse_unary_expression();
        node->data_type = node->left->data_type;
        return node;
    }
    
    return parse_primary_expression();
}

// 解析乘法表达式
ASTNode* parse_multiplicative_expression() {
    ASTNode* left = parse_unary_expression();
    
    while (current_token.type == TOKEN_STAR || 
           current_token.type == TOKEN_SLASH ||
           current_token.type == TOKEN_PERCENT) {
        
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, current_token.value);
        node->left = left;
        advance_token();
        node->right = parse_unary_expression();
        
        // 类型提升
        if (left->data_type == TYPE_FLOAT || node->right->data_type == TYPE_FLOAT) {
            node->data_type = TYPE_FLOAT;
        } else {
            node->data_type = TYPE_INT;
        }
        
        left = node;
    }
    
    return left;
}

// 解析加法表达式
ASTNode* parse_additive_expression() {
    ASTNode* left = parse_multiplicative_expression();
    
    while (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS) {
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, current_token.value);
        node->left = left;
        advance_token();
        node->right = parse_multiplicative_expression();
        
        // 类型提升
        if (left->data_type == TYPE_FLOAT || node->right->data_type == TYPE_FLOAT) {
            node->data_type = TYPE_FLOAT;
        } else {
            node->data_type = TYPE_INT;
        }
        
        left = node;
    }
    
    return left;
}

// 解析关系表达式
ASTNode* parse_relational_expression() {
    ASTNode* left = parse_additive_expression();
    
    while (current_token.type == TOKEN_LT || current_token.type == TOKEN_GT ||
           current_token.type == TOKEN_LE || current_token.type == TOKEN_GE) {
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, current_token.value);
        node->left = left;
        advance_token();
        node->right = parse_additive_expression();
        node->data_type = TYPE_INT; // 比较结果为int (0或1)
        left = node;
    }
    
    return left;
}

// 解析相等表达式
ASTNode* parse_equality_expression() {
    ASTNode* left = parse_relational_expression();
    
    while (current_token.type == TOKEN_EQ || current_token.type == TOKEN_NE) {
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, current_token.value);
        node->left = left;
        advance_token();
        node->right = parse_relational_expression();
        node->data_type = TYPE_INT;
        left = node;
    }
    
    return left;
}

// 解析逻辑与表达式
ASTNode* parse_logical_and_expression() {
    ASTNode* left = parse_equality_expression();
    
    while (current_token.type == TOKEN_AND) {
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, "&&");
        node->left = left;
        advance_token();
        node->right = parse_equality_expression();
        node->data_type = TYPE_INT;
        left = node;
    }
    
    return left;
}

// 解析逻辑或表达式
ASTNode* parse_logical_or_expression() {
    ASTNode* left = parse_logical_and_expression();
    
    while (current_token.type == TOKEN_OR) {
        ASTNode* node = create_node(NODE_BINARY_EXPR);
        strcpy(node->name, "||");
        node->left = left;
        advance_token();
        node->right = parse_logical_and_expression();
        node->data_type = TYPE_INT;
        left = node;
    }
    
    return left;
}

// 解析赋值表达式
ASTNode* parse_assignment_expression() {
    ASTNode* left = parse_logical_or_expression();
    
    if (current_token.type == TOKEN_ASSIGN) {
        ASTNode* node = create_node(NODE_ASSIGN_EXPR);
        node->left = left;
        advance_token();
        node->right = parse_assignment_expression();
        node->data_type = left->data_type;
        return node;
    }
    
    return left;
}

// 解析表达式
ASTNode* parse_expression() {
    return parse_assignment_expression();
}

// 解析变量声明
ASTNode* parse_var_declaration() {
    DataType type = get_data_type(current_token.type);
    advance_token();
    
    if (current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error at line %d: Expected identifier\n", current_token.line);
        return NULL;
    }
    
    ASTNode* node = create_node(NODE_VAR_DECL);
    strcpy(node->name, current_token.value);
    node->data_type = type;
    advance_token();
    
    // 添加到符号表
    add_symbol(node->name, type, SYMBOL_VARIABLE);
    
    // 初始化表达式
    if (current_token.type == TOKEN_ASSIGN) {
        advance_token();
        node->initializer = parse_expression();
    }
    
    expect_token(TOKEN_SEMICOLON);
    
    return node;
}

// 解析if语句
ASTNode* parse_if_statement() {
    ASTNode* node = create_node(NODE_IF_STMT);
    
    advance_token(); // if
    expect_token(TOKEN_LPAREN);
    node->condition = parse_expression();
    expect_token(TOKEN_RPAREN);
    
    node->then_branch = parse_statement();
    
    if (current_token.type == TOKEN_ELSE) {
        advance_token();
        node->else_branch = parse_statement();
    }
    
    return node;
}

// 解析while语句
ASTNode* parse_while_statement() {
    ASTNode* node = create_node(NODE_WHILE_STMT);
    
    advance_token(); // while
    expect_token(TOKEN_LPAREN);
    node->condition = parse_expression();
    expect_token(TOKEN_RPAREN);
    node->body = parse_statement();
    
    return node;
}

// 解析for语句
ASTNode* parse_for_statement() {
    ASTNode* node = create_node(NODE_FOR_STMT);
    
    advance_token(); // for
    expect_token(TOKEN_LPAREN);
    
    // 初始化
    if (current_token.type != TOKEN_SEMICOLON) {
        if (current_token.type == TOKEN_INT || current_token.type == TOKEN_FLOAT ||
            current_token.type == TOKEN_CHAR) {
            node->init = parse_var_declaration();
        } else {
            node->init = create_node(NODE_EXPR_STMT);
            node->init->left = parse_expression();
            expect_token(TOKEN_SEMICOLON);
        }
    } else {
        advance_token();
    }
    
    // 条件
    if (current_token.type != TOKEN_SEMICOLON) {
        node->condition = parse_expression();
    }
    expect_token(TOKEN_SEMICOLON);
    
    // 更新
    if (current_token.type != TOKEN_RPAREN) {
        node->update = create_node(NODE_EXPR_STMT);
        node->update->left = parse_expression();
    }
    expect_token(TOKEN_RPAREN);
    
    node->body = parse_statement();
    
    return node;
}

// 解析return语句
ASTNode* parse_return_statement() {
    ASTNode* node = create_node(NODE_RETURN_STMT);
    
    advance_token(); // return
    
    if (current_token.type != TOKEN_SEMICOLON) {
        node->left = parse_expression();
    }
    
    expect_token(TOKEN_SEMICOLON);
    
    return node;
}

// 解析复合语句
ASTNode* parse_compound_statement() {
    ASTNode* node = create_node(NODE_COMPOUND_STMT);
    
    expect_token(TOKEN_LBRACE);
    enter_scope();
    
    node->stmt_count = 0;
    while (current_token.type != TOKEN_RBRACE && current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_INT || current_token.type == TOKEN_FLOAT ||
            current_token.type == TOKEN_CHAR) {
            // 变量声明
            ASTNode* decl = parse_var_declaration();
            if (decl) {
                node->statements[node->stmt_count++] = decl;
            }
        } else {
            // 语句
            ASTNode* stmt = parse_statement();
            if (stmt) {
                node->statements[node->stmt_count++] = stmt;
            }
        }
    }
    
    expect_token(TOKEN_RBRACE);
    leave_scope();
    
    return node;
}

// 解析语句
ASTNode* parse_statement() {
    switch (current_token.type) {
        case TOKEN_IF:
            return parse_if_statement();
            
        case TOKEN_WHILE:
            return parse_while_statement();
            
        case TOKEN_FOR:
            return parse_for_statement();
            
        case TOKEN_RETURN:
            return parse_return_statement();
            
        case TOKEN_BREAK:
            advance_token();
            expect_token(TOKEN_SEMICOLON);
            return create_node(NODE_BREAK_STMT);
            
        case TOKEN_CONTINUE:
            advance_token();
            expect_token(TOKEN_SEMICOLON);
            return create_node(NODE_CONTINUE_STMT);
            
        case TOKEN_LBRACE:
            return parse_compound_statement();
            
        case TOKEN_SEMICOLON:
            advance_token();
            return NULL;
            
        default:
            // 表达式语句
            if (current_token.type == TOKEN_IDENTIFIER || 
                current_token.type == TOKEN_INTEGER ||
                current_token.type == TOKEN_FLOAT_NUM ||
                current_token.type == TOKEN_LPAREN) {
                
                ASTNode* node = create_node(NODE_EXPR_STMT);
                node->left = parse_expression();
                expect_token(TOKEN_SEMICOLON);
                return node;
            }
            
            fprintf(stderr, "Error at line %d: Unexpected token '%s'\n",
                    current_token.line, get_token_name(current_token.type));
            advance_token();
            return NULL;
    }
}

// 解析函数声明
ASTNode* parse_function_declaration() {
    DataType return_type = get_data_type(current_token.type);
    advance_token();
    
    if (current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Error at line %d: Expected function name\n", current_token.line);
        return NULL;
    }
    
    ASTNode* node = create_node(NODE_FUNCTION_DECL);
    strcpy(node->name, current_token.value);
    node->data_type = return_type;
    advance_token();
    
    expect_token(TOKEN_LPAREN);
    
    // 解析参数列表
    node->param_count = 0;
    DataType param_types[MAX_PARAMS];
    
    if (current_token.type != TOKEN_RPAREN) {
        do {
            if (current_token.type == TOKEN_INT || current_token.type == TOKEN_FLOAT ||
                current_token.type == TOKEN_CHAR) {
                
                DataType param_type = get_data_type(current_token.type);
                advance_token();
                
                if (current_token.type == TOKEN_IDENTIFIER) {
                    ASTNode* param = create_node(NODE_PARAM);
                    strcpy(param->name, current_token.value);
                    param->data_type = param_type;
                    node->params[node->param_count] = param;
                    param_types[node->param_count] = param_type;
                    node->param_count++;
                    advance_token();
                }
                
                if (current_token.type == TOKEN_COMMA) {
                    advance_token();
                } else {
                    break;
                }
            }
        } while (node->param_count < MAX_PARAMS);
    }
    
    expect_token(TOKEN_RPAREN);
    
    // 添加函数到符号表
    add_function_symbol(node->name, return_type, node->param_count, param_types);
    
    // 解析函数体
    if (current_token.type == TOKEN_LBRACE) {
        node->body = parse_compound_statement();
    } else {
        fprintf(stderr, "Error at line %d: Expected function body\n", current_token.line);
        return NULL;
    }
    
    return node;
}

// 解析程序
ASTNode* parse_program() {
    ASTNode* program = create_node(NODE_PROGRAM);
    program->stmt_count = 0;
    
    while (current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_INT || current_token.type == TOKEN_FLOAT ||
            current_token.type == TOKEN_CHAR || current_token.type == TOKEN_VOID) {
            
            // 检查是否为函数声明
            if (next_token.type == TOKEN_IDENTIFIER && 
                (next_token.type == TOKEN_LPAREN || 
                 (source_code[source_pos] == '('))) {
                ASTNode* func = parse_function_declaration();
                if (func) {
                    program->statements[program->stmt_count++] = func;
                }
            } else {
                // 全局变量声明
                ASTNode* var = parse_var_declaration();
                if (var) {
                    program->statements[program->stmt_count++] = var;
                }
            }
        } else {
            fprintf(stderr, "Error at line %d: Expected declaration\n", current_token.line);
            advance_token();
        }
    }
    
    return program;
}

/* ==================== 语义分析 ==================== */

// 类型检查
bool check_types(DataType left, DataType right, const char* op) {
    // 允许int和float之间的运算
    if ((left == TYPE_INT || left == TYPE_FLOAT) &&
        (right == TYPE_INT || right == TYPE_FLOAT)) {
        return true;
    }
    
    // 允许char和int之间的运算
    if ((left == TYPE_CHAR && right == TYPE_INT) ||
        (left == TYPE_INT && right == TYPE_CHAR)) {
        return true;
    }
    
    if (left != right) {
        fprintf(stderr, "Warning: Type mismatch in operation '%s'\n", op);
    }
    
    return true;
}

// 语义分析AST
void analyze_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->stmt_count; i++) {
                analyze_ast(node->statements[i]);
            }
            break;
            
        case NODE_FUNCTION_DECL:
            enter_scope();
            // 添加参数到符号表
            for (int i = 0; i < node->param_count; i++) {
                add_symbol(node->params[i]->name, node->params[i]->data_type, SYMBOL_PARAMETER);
            }
            analyze_ast(node->body);
            leave_scope();
            break;
            
        case NODE_COMPOUND_STMT:
            for (int i = 0; i < node->stmt_count; i++) {
                analyze_ast(node->statements[i]);
            }
            break;
            
        case NODE_VAR_DECL:
            if (node->initializer) {
                analyze_ast(node->initializer);
                check_types(node->data_type, node->initializer->data_type, "=");
            }
            break;
            
        case NODE_IF_STMT:
            analyze_ast(node->condition);
            analyze_ast(node->then_branch);
            analyze_ast(node->else_branch);
            break;
            
        case NODE_WHILE_STMT:
            analyze_ast(node->condition);
            analyze_ast(node->body);
            break;
            
        case NODE_FOR_STMT:
            analyze_ast(node->init);
            analyze_ast(node->condition);
            analyze_ast(node->update);
            analyze_ast(node->body);
            break;
            
        case NODE_RETURN_STMT:
            if (node->left) {
                analyze_ast(node->left);
            }
            break;
            
        case NODE_EXPR_STMT:
            analyze_ast(node->left);
            break;
            
        case NODE_ASSIGN_EXPR:
            analyze_ast(node->left);
            analyze_ast(node->right);
            check_types(node->left->data_type, node->right->data_type, "=");
            break;
            
        case NODE_BINARY_EXPR:
            analyze_ast(node->left);
            analyze_ast(node->right);
            check_types(node->left->data_type, node->right->data_type, node->name);
            break;
            
        case NODE_UNARY_EXPR:
            analyze_ast(node->left);
            break;
            
        case NODE_CALL_EXPR:
            // 检查函数是否存在
            {
                Symbol* func = lookup_symbol(node->name);
                if (!func) {
                    fprintf(stderr, "Error: Function '%s' not declared\n", node->name);
                } else if (func->kind != SYMBOL_FUNCTION) {
                    fprintf(stderr, "Error: '%s' is not a function\n", node->name);
                } else {
                    node->data_type = func->type;
                    // 检查参数数量
                    if (node->arg_count != func->param_count) {
                        fprintf(stderr, "Error: Function '%s' expects %d arguments, got %d\n",
                                node->name, func->param_count, node->arg_count);
                    }
                }
            }
            for (int i = 0; i < node->arg_count; i++) {
                analyze_ast(node->args[i]);
            }
            break;
            
        case NODE_IDENTIFIER:
            {
                Symbol* sym = lookup_symbol(node->name);
                if (!sym) {
                    fprintf(stderr, "Error: Variable '%s' not declared\n", node->name);
                } else {
                    node->data_type = sym->type;
                    node->offset = sym->offset;
                }
            }
            break;
            
        default:
            break;
    }
}

/* ==================== 代码生成器 ==================== */

// 追加代码
void emit_code(const char* format, ...) {
    va_list args;
    va_start(args, format);
    code_pos += vsprintf(generated_code + code_pos, format, args);
    va_end(args);
}

// 生成新标签
int new_label() {
    return label_counter++;
}

// 生成表达式代码
void generate_expression(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_INTEGER:
            emit_code("    push %d\n", node->int_value);
            break;
            
        case NODE_FLOAT_NUM:
            emit_code("    ; float %f\n", node->float_value);
            emit_code("    push __float_%d\n", temp_var_counter++);
            break;
            
        case NODE_CHAR_LITERAL:
            emit_code("    push %d\n", (int)node->char_value);
            break;
            
        case NODE_IDENTIFIER:
            {
                Symbol* sym = lookup_symbol(node->name);
                if (sym) {
                    emit_code("    mov eax, [ebp-%d]  ; load %s\n", 
                              sym->offset + 4, node->name);
                    emit_code("    push eax\n");
                }
            }
            break;
            
        case NODE_BINARY_EXPR:
            generate_expression(node->right);
            generate_expression(node->left);
            
            emit_code("    pop eax\n");
            emit_code("    pop ebx\n");
            
            if (strcmp(node->name, "+") == 0) {
                emit_code("    add eax, ebx\n");
            } else if (strcmp(node->name, "-") == 0) {
                emit_code("    sub eax, ebx\n");
            } else if (strcmp(node->name, "*") == 0) {
                emit_code("    imul eax, ebx\n");
            } else if (strcmp(node->name, "/") == 0) {
                emit_code("    cdq\n");
                emit_code("    idiv ebx\n");
            } else if (strcmp(node->name, "%") == 0) {
                emit_code("    cdq\n");
                emit_code("    idiv ebx\n");
                emit_code("    mov eax, edx\n");
            } else if (strcmp(node->name, "==") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    sete al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, "!=") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    setne al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, "<") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    setl al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, ">") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    setg al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, "<=") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    setle al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, ">=") == 0) {
                emit_code("    cmp eax, ebx\n");
                emit_code("    setge al\n");
                emit_code("    movzx eax, al\n");
            } else if (strcmp(node->name, "&&") == 0) {
                emit_code("    and eax, ebx\n");
            } else if (strcmp(node->name, "||") == 0) {
                emit_code("    or eax, ebx\n");
            }
            
            emit_code("    push eax\n");
            break;
            
        case NODE_UNARY_EXPR:
            generate_expression(node->left);
            emit_code("    pop eax\n");
            
            if (strcmp(node->name, "-") == 0) {
                emit_code("    neg eax\n");
            } else if (strcmp(node->name, "!") == 0) {
                emit_code("    test eax, eax\n");
                emit_code("    setz al\n");
                emit_code("    movzx eax, al\n");
            }
            
            emit_code("    push eax\n");
            break;
            
        case NODE_ASSIGN_EXPR:
            generate_expression(node->right);
            {
                Symbol* sym = lookup_symbol(node->left->name);
                if (sym) {
                    emit_code("    pop eax\n");
                    emit_code("    mov [ebp-%d], eax  ; store %s\n", 
                              sym->offset + 4, node->left->name);
                    emit_code("    push eax\n");
                }
            }
            break;
            
        case NODE_CALL_EXPR:
            // 保存参数
            for (int i = node->arg_count - 1; i >= 0; i--) {
                generate_expression(node->args[i]);
            }
            emit_code("    call %s\n", node->name);
            emit_code("    add esp, %d\n", node->arg_count * 4);
            emit_code("    push eax\n");
            break;
            
        default:
            break;
    }
}

// 生成语句代码
void generate_statement(ASTNode* node) {
    if (!node) return;
    
    int l1, l2, l3;
    
    switch (node->type) {
        case NODE_COMPOUND_STMT:
            for (int i = 0; i < node->stmt_count; i++) {
                generate_statement(node->statements[i]);
            }
            break;
            
        case NODE_VAR_DECL:
            if (node->initializer) {
                generate_expression(node->initializer);
                emit_code("    pop eax\n");
                emit_code("    mov [ebp-%d], eax  ; init %s\n", 
                          node->offset + 4, node->name);
            }
            break;
            
        case NODE_IF_STMT:
            l1 = new_label();
            l2 = new_label();
            
            generate_expression(node->condition);
            emit_code("    pop eax\n");
            emit_code("    test eax, eax\n");
            emit_code("    jz .L%d\n", l1);
            
            generate_statement(node->then_branch);
            emit_code("    jmp .L%d\n", l2);
            
            emit_code(".L%d:\n", l1);
            if (node->else_branch) {
                generate_statement(node->else_branch);
            }
            emit_code(".L%d:\n", l2);
            break;
            
        case NODE_WHILE_STMT:
            l1 = new_label();
            l2 = new_label();
            
            emit_code(".L%d:  ; while start\n", l1);
            generate_expression(node->condition);
            emit_code("    pop eax\n");
            emit_code("    test eax, eax\n");
            emit_code("    jz .L%d\n", l2);
            
            generate_statement(node->body);
            emit_code("    jmp .L%d\n", l1);
            
            emit_code(".L%d:  ; while end\n", l2);
            break;
            
        case NODE_FOR_STMT:
            l1 = new_label();
            l2 = new_label();
            
            if (node->init) {
                generate_statement(node->init);
            }
            
            emit_code(".L%d:  ; for start\n", l1);
            if (node->condition) {
                generate_expression(node->condition);
                emit_code("    pop eax\n");
                emit_code("    test eax, eax\n");
                emit_code("    jz .L%d\n", l2);
            }
            
            generate_statement(node->body);
            
            if (node->update) {
                generate_expression(node->update);
                emit_code("    add esp, 4\n");  // 丢弃结果
            }
            emit_code("    jmp .L%d\n", l1);
            
            emit_code(".L%d:  ; for end\n", l2);
            break;
            
        case NODE_RETURN_STMT:
            if (node->left) {
                generate_expression(node->left);
                emit_code("    pop eax\n");
            }
            emit_code("    mov esp, ebp\n");
            emit_code("    pop ebp\n");
            emit_code("    ret\n");
            break;
            
        case NODE_EXPR_STMT:
            generate_expression(node->left);
            if (node->left) {
                emit_code("    add esp, 4\n");  // 丢弃表达式结果
            }
            break;
            
        case NODE_BREAK_STMT:
            emit_code("    jmp .L%d  ; break\n", label_counter - 1);
            break;
            
        case NODE_CONTINUE_STMT:
            emit_code("    jmp .L%d  ; continue\n", label_counter - 2);
            break;
            
        default:
            break;
    }
}

// 生成函数代码
void generate_function(ASTNode* node) {
    emit_code("\n%s:\n", node->name);
    emit_code("    push ebp\n");
    emit_code("    mov ebp, esp\n");
    
    // 分配局部变量空间
    int local_size = 0;
    // 简化处理：假设每个局部变量4字节
    emit_code("    sub esp, %d\n", MAX_LOCAL_VARS * 4);
    
    // 保存参数
    for (int i = 0; i < node->param_count; i++) {
        Symbol* sym = lookup_symbol(node->params[i]->name);
        if (sym) {
            emit_code("    mov eax, [ebp+%d]  ; param %s\n", 
                      8 + i * 4, node->params[i]->name);
            emit_code("    mov [ebp-%d], eax\n", sym->offset + 4);
        }
    }
    
    generate_statement(node->body);
    
    // 默认返回
    emit_code("    mov eax, 0\n");
    emit_code("    mov esp, ebp\n");
    emit_code("    pop ebp\n");
    emit_code("    ret\n");
}

// 生成完整程序代码
void generate_program(ASTNode* program) {
    emit_code("; Generated by Simple C Compiler\n");
    emit_code("; Target: x86 Assembly (NASM syntax)\n\n");
    emit_code("section .data\n");
    emit_code("    ; Data section for string literals and global variables\n\n");
    emit_code("section .text\n");
    emit_code("    global _start\n");
    emit_code("    global main\n\n");
    
    emit_code("_start:\n");
    emit_code("    call main\n");
    emit_code("    mov ebx, eax\n");
    emit_code("    mov eax, 1\n");
    emit_code("    int 0x80\n\n");
    
    for (int i = 0; i < program->stmt_count; i++) {
        ASTNode* node = program->statements[i];
        if (node->type == NODE_FUNCTION_DECL) {
            generate_function(node);
        }
    }
}

/* ==================== AST打印 ==================== */

void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int i = 0; i < node->stmt_count; i++) {
                print_ast(node->statements[i], indent + 1);
            }
            break;
            
        case NODE_FUNCTION_DECL:
            printf("FunctionDecl: %s (returns %s)\n", 
                   node->name, get_type_name(node->data_type));
            print_indent(indent + 1);
            printf("Parameters:\n");
            for (int i = 0; i < node->param_count; i++) {
                print_ast(node->params[i], indent + 2);
            }
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->body, indent + 2);
            break;
            
        case NODE_PARAM:
            printf("Param: %s (%s)\n", node->name, get_type_name(node->data_type));
            break;
            
        case NODE_VAR_DECL:
            printf("VarDecl: %s (%s)\n", node->name, get_type_name(node->data_type));
            if (node->initializer) {
                print_indent(indent + 1);
                printf("Initializer:\n");
                print_ast(node->initializer, indent + 2);
            }
            break;
            
        case NODE_COMPOUND_STMT:
            printf("CompoundStmt\n");
            for (int i = 0; i < node->stmt_count; i++) {
                print_ast(node->statements[i], indent + 1);
            }
            break;
            
        case NODE_IF_STMT:
            printf("IfStmt\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->condition, indent + 2);
            print_indent(indent + 1);
            printf("Then:\n");
            print_ast(node->then_branch, indent + 2);
            if (node->else_branch) {
                print_indent(indent + 1);
                printf("Else:\n");
                print_ast(node->else_branch, indent + 2);
            }
            break;
            
        case NODE_WHILE_STMT:
            printf("WhileStmt\n");
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->condition, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->body, indent + 2);
            break;
            
        case NODE_FOR_STMT:
            printf("ForStmt\n");
            print_indent(indent + 1);
            printf("Init:\n");
            print_ast(node->init, indent + 2);
            print_indent(indent + 1);
            printf("Condition:\n");
            print_ast(node->condition, indent + 2);
            print_indent(indent + 1);
            printf("Update:\n");
            print_ast(node->update, indent + 2);
            print_indent(indent + 1);
            printf("Body:\n");
            print_ast(node->body, indent + 2);
            break;
            
        case NODE_RETURN_STMT:
            printf("ReturnStmt\n");
            if (node->left) {
                print_ast(node->left, indent + 1);
            }
            break;
            
        case NODE_EXPR_STMT:
            printf("ExprStmt\n");
            print_ast(node->left, indent + 1);
            break;
            
        case NODE_ASSIGN_EXPR:
            printf("AssignExpr: =\n");
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(node->left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(node->right, indent + 2);
            break;
            
        case NODE_BINARY_EXPR:
            printf("BinaryExpr: %s\n", node->name);
            print_indent(indent + 1);
            printf("Left:\n");
            print_ast(node->left, indent + 2);
            print_indent(indent + 1);
            printf("Right:\n");
            print_ast(node->right, indent + 2);
            break;
            
        case NODE_UNARY_EXPR:
            printf("UnaryExpr: %s\n", node->name);
            print_ast(node->left, indent + 1);
            break;
            
        case NODE_CALL_EXPR:
            printf("CallExpr: %s\n", node->name);
            for (int i = 0; i < node->arg_count; i++) {
                print_indent(indent + 1);
                printf("Arg %d:\n", i);
                print_ast(node->args[i], indent + 2);
            }
            break;
            
        case NODE_IDENTIFIER:
            printf("Identifier: %s (%s)\n", node->name, get_type_name(node->data_type));
            break;
            
        case NODE_INTEGER:
            printf("Integer: %d\n", node->int_value);
            break;
            
        case NODE_FLOAT_NUM:
            printf("Float: %f\n", node->float_value);
            break;
            
        case NODE_CHAR_LITERAL:
            printf("Char: '%c'\n", node->char_value);
            break;
            
        case NODE_STRING_LITERAL:
            printf("String: \"%s\"\n", node->name);
            break;
            
        default:
            printf("Unknown node type: %d\n", node->type);
            break;
    }
}

/* ==================== 主程序 ==================== */

void print_usage(const char* program_name) {
    printf("Simple C Compiler\n");
    printf("Usage: %s <source_file.c> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -o <output_file>  Specify output file name\n");
    printf("  --ast             Print AST (Abstract Syntax Tree)\n");
    printf("  --tokens          Print tokens\n");
    printf("  --help            Show this help message\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* input_file = NULL;
    const char* output_file = "output.asm";
    bool print_ast_flag = false;
    bool print_tokens_flag = false;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--ast") == 0) {
            print_ast_flag = true;
        } else if (strcmp(argv[i], "--tokens") == 0) {
            print_tokens_flag = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        return 1;
    }
    
    // 加载源代码
    if (!load_source(input_file)) {
        return 1;
    }
    
    // 初始化
    init_symbol_table();
    init_lexer();
    
    // 打印Token
    if (print_tokens_flag) {
        printf("=== Tokens ===\n");
        Token t = current_token;
        while (t.type != TOKEN_EOF) {
            printf("Line %d: %-15s '%s'\n", t.line, get_token_name(t.type), t.value);
            t = get_next_token();
        }
        printf("\n");
        
        // 重新初始化
        source_pos = 0;
        current_line = 1;
        current_column = 1;
        init_lexer();
    }
    
    // 解析
    printf("=== Parsing ===\n");
    ASTNode* program = parse_program();
    
    if (!program) {
        fprintf(stderr, "Error: Failed to parse program\n");
        return 1;
    }
    
    printf("Parsing completed successfully.\n\n");
    
    // 打印AST
    if (print_ast_flag) {
        printf("=== Abstract Syntax Tree ===\n");
        print_ast(program, 0);
        printf("\n");
    }
    
    // 语义分析
    printf("=== Semantic Analysis ===\n");
    analyze_ast(program);
    printf("Semantic analysis completed.\n\n");
    
    // 代码生成
    printf("=== Code Generation ===\n");
    generate_program(program);
    printf("Code generation completed.\n\n");
    
    // 保存生成的代码
    FILE* out = fopen(output_file, "w");
    if (out) {
        fprintf(out, "%s", generated_code);
        fclose(out);
        printf("Output saved to: %s\n", output_file);
    } else {
        fprintf(stderr, "Error: Cannot write to output file\n");
        return 1;
    }
    
    // 清理
    for (int i = 0; i < ast_node_count; i++) {
        free(ast_nodes[i]);
    }
    
    return 0;
}
