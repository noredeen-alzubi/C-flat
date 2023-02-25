#ifndef CFLAT_H
#define CFLAT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define FOREACH_KEYWORD_TYPE(KEYWORD_TYPE)           \
        KEYWORD_TYPE(TK_AUTO, "auto")              \
        KEYWORD_TYPE(TK_TYPEDEF, "typedef")              \
        KEYWORD_TYPE(TK_BREAK, "break")            \
        KEYWORD_TYPE(TK_CASE, "case")              \
        KEYWORD_TYPE(TK_KW_CHAR, "char")              \
        KEYWORD_TYPE(TK_INT, "int")                \
        KEYWORD_TYPE(TK_DOUBLE, "double")          \
        KEYWORD_TYPE(TK_FLOAT, "float")            \
        KEYWORD_TYPE(TK_SIGNED, "signed")          \
        KEYWORD_TYPE(TK_UNSIGNED, "unsigned")      \
        KEYWORD_TYPE(TK_VOID, "unsigned")          \
        KEYWORD_TYPE(TK_ENUM, "enum")              \
        KEYWORD_TYPE(TK_CONST, "const")            \
        KEYWORD_TYPE(TK_EXTERN, "extern")          \
        KEYWORD_TYPE(TK_STATIC, "static")          \
        KEYWORD_TYPE(TK_CONTINUE, "continue")      \
        KEYWORD_TYPE(TK_DEFAULT, "default")        \
        KEYWORD_TYPE(TK_DO, "do")                  \
        KEYWORD_TYPE(TK_ELSE, "else")              \
        KEYWORD_TYPE(TK_IF, "if")                  \
        KEYWORD_TYPE(TK_RESTRICT, "restrict")                  \
        KEYWORD_TYPE(TK_VOLATILE, "volatile")                  \
        KEYWORD_TYPE(TK__ATOMIC, "_Atomic")                  \
        KEYWORD_TYPE(TK__GENERIC, "_Generic")                  \

#define KEYWORD_TYPE_COUNT 24

// IMPORTANT: multi-char punctuators come first in the list
#define FOREACH_PUNCTUATOR_TYPE(PUNCTUATOR_TYPE)        \
        PUNCTUATOR_TYPE(TK_DEREF, "->")          \
        PUNCTUATOR_TYPE(TK_LNOTEQ, "!=")         \
        PUNCTUATOR_TYPE(TK_INC, "+=")            \
        PUNCTUATOR_TYPE(TK_DEC, "-=")            \
        PUNCTUATOR_TYPE(TK_INC_ONE, "++")            \
        PUNCTUATOR_TYPE(TK_DEC_ONE, "--")            \
        PUNCTUATOR_TYPE(TK_SEMICOLON, ";")       \
        PUNCTUATOR_TYPE(TK_LPAREN, "(")          \
        PUNCTUATOR_TYPE(TK_RPAREN, ")")          \
        PUNCTUATOR_TYPE(TK_LBRACE, "{")          \
        PUNCTUATOR_TYPE(TK_RBRACE, "}")          \
        PUNCTUATOR_TYPE(TK_LBRACKET, "[")        \
        PUNCTUATOR_TYPE(TK_RBRACKET, "]")        \
        PUNCTUATOR_TYPE(TK_DOT, ".")             \
        PUNCTUATOR_TYPE(TK_ADDR, "&")            \
        PUNCTUATOR_TYPE(TK_STAR, "*")            \
        PUNCTUATOR_TYPE(TK_PLUS, "+")            \
        PUNCTUATOR_TYPE(TK_EQ, "=")            \
        PUNCTUATOR_TYPE(TK_MINUS, "-")           \
        PUNCTUATOR_TYPE(TK_BNOT, "~")            \
        PUNCTUATOR_TYPE(TK_LNOT, "!")            \

#define PUNCTUATOR_TYPE_COUNT 21

// TODO: get rid of this
#define ALPHABET_SIZE 128

#define GENERATE_ENUM(ENUM, TEXT) ENUM,
#define GENERATE_STRING(ENUM, TEXT) TEXT,

typedef enum {
    TK_ID, TK_NUM, TK_STR, TK_CHAR, TK_EOF,
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} TokenType;

typedef struct string dstring;
struct string {
    char* str;
    size_t size, len;
};

void dstring_append(dstring* dstr, char c);
char dstring_at(dstring* dstr, int i);
char dstring_set(dstring* dstr, int i, char c);
void dstring_initialize(dstring* dstr);
void dstring_initialize_str(dstring* dstr, char* str, int n);
void dstring_cat(dstring* dest, dstring* src);
void dstring_reserve(dstring* dstr, size_t len);
void dstring_free(dstring dstr);

// lex.c

typedef struct Token Token;
struct Token {
    TokenType ty;
    Token* next;
    dstring text; // for debugging

    int64_t i_value; // TK_NUM, TK_CHAR
    long double f_value; // TK_NUM
    dstring s_value; // TK_STR
};


typedef struct TokenTrieNode TokenTrieNode;
struct TokenTrieNode {
    char ch;
    TokenTrieNode* children[ALPHABET_SIZE];
    Token* token;
};

TokenTrieNode* build_token_trie(char** strings, int* subtypes, size_t count);

// type.c

typedef enum {
    TY_INT,
    TY_CHAR,
    TY_DOUBLE,
    TY_PTR,
    TY_STRUCT,
    TY_ENUM
} TypeKind;


typedef struct Type Type;
struct Type {
    TypeKind kind;
    size_t size;
    int alignment; // compiler-set alignment
    bool is_unsigned;
    bool is_const;

    Type *target_ty; // if kind == TY_PTR
};

typedef struct {
    bool is_typedef;
    bool is_static;
    bool is_extern;
    bool is_inline;
    int alignment;
} VarAttrs;

// parse.c

typedef struct Obj Obj;
struct Obj {
    Type *ty;
    VarAttrs *attrs;
    char *id;

    int param_count;
    Obj **params;
};

typedef struct {
    char *id;
    Obj *var;
} VarRef;

typedef struct {
    char *id;
    Obj *func;
} FuncInvok;


typedef struct Expr Expr;
struct Expr {
    Expr *l_operand;
    Expr *r_operand;

    VarRef var_ref; // Literals put in compiler-generated variables
    FuncInvok func_invok;
};

#endif
