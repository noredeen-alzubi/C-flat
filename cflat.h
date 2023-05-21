#ifndef CFLAT_H
#define CFLAT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define FOREACH_KEYWORD_TYPE(KEYWORD_TYPE)         \
        KEYWORD_TYPE(TK_AUTO, "auto")              \
        KEYWORD_TYPE(TK_TYPEDEF, "typedef")        \
        KEYWORD_TYPE(TK_BREAK, "break")            \
        KEYWORD_TYPE(TK_CASE, "case")              \
        KEYWORD_TYPE(TK_KW_CHAR, "char")           \
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
        KEYWORD_TYPE(TK_RESTRICT, "restrict")      \
        KEYWORD_TYPE(TK_VOLATILE, "volatile")      \
        KEYWORD_TYPE(TK__ATOMIC, "_Atomic")        \
        KEYWORD_TYPE(TK__GENERIC, "_Generic")      \

#define KEYWORD_TYPE_COUNT 24

// IMPORTANT: multi-char punctuators come first in the list
#define FOREACH_PUNCTUATOR_TYPE(PUNCTUATOR_TYPE)        \
        PUNCTUATOR_TYPE(TK_DEREF, "->")                 \
        PUNCTUATOR_TYPE(TK_LNOTEQ, "!=")                \
        PUNCTUATOR_TYPE(TK_INC, "+=")                   \
        PUNCTUATOR_TYPE(TK_DEC, "-=")                   \
        PUNCTUATOR_TYPE(TK_INC_ONE, "++")               \
        PUNCTUATOR_TYPE(TK_DEC_ONE, "--")               \
        PUNCTUATOR_TYPE(TK_SEMICOLON, ";")              \
        PUNCTUATOR_TYPE(TK_LPAREN, "(")                 \
        PUNCTUATOR_TYPE(TK_RPAREN, ")")                 \
        PUNCTUATOR_TYPE(TK_LBRACE, "{")                 \
        PUNCTUATOR_TYPE(TK_RBRACE, "}")                 \
        PUNCTUATOR_TYPE(TK_LBRACKET, "[")               \
        PUNCTUATOR_TYPE(TK_RBRACKET, "]")               \
        PUNCTUATOR_TYPE(TK_DOT, ".")                    \
        PUNCTUATOR_TYPE(TK_COMMA, ",")                    \
        PUNCTUATOR_TYPE(TK_ADDR, "&")                   \
        PUNCTUATOR_TYPE(TK_STAR, "*")                   \
        PUNCTUATOR_TYPE(TK_PLUS, "+")                   \
        PUNCTUATOR_TYPE(TK_EQ, "=")                     \
        PUNCTUATOR_TYPE(TK_MINUS, "-")                  \
        PUNCTUATOR_TYPE(TK_BNOT, "~")                   \
        PUNCTUATOR_TYPE(TK_LNOT, "!")                   \

#define PUNCTUATOR_TYPE_COUNT 22

// TODO: get rid of this
#define ALPHABET_SIZE 128

#define GENERATE_ENUM(ENUM, TEXT) ENUM,
#define GENERATE_STRING(ENUM, TEXT) TEXT,

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

typedef enum {
    TK_ID, TK_NUM, TK_STR, TK_CHAR, TK_EOF,
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} TokenType;

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
    TY_ARR,
    TY_STRUCT,
    TY_ENUM,
    TY_FUNC,
    TY_VOID,
} TypeKind;

typedef struct Type Type;
typedef struct VarAttrs VarAttrs;
typedef struct Member Member;

struct Type {
    TypeKind kind;
    size_t size;
    bool is_unsigned;
    bool is_const;

    // TY_ARR
    int arr_len;

    // TY_PTR
    Type *target_ty;

    // TY_FUNC
    Type *ret_ty;

    // TY_STRUCT
    Member *members;
};

struct VarAttrs {
    bool is_typedef;
    bool is_static;
    bool is_extern;
    bool is_inline;
    int alignment;
};

// parse.c

typedef enum { BI_BIS, BI_VAR, BI_EXPR, BI_COND, BI_ITER, BI_JMP } BlockItemType;
typedef enum { JMP_GOTO, JMP_CONT, JMP_BREAK, JMP_RET } JumpType;

typedef struct BlockItem BlockItem;
typedef struct Obj Obj;
typedef struct VarRef VarRef;
typedef struct FuncInvok FuncInvok;
typedef struct Expr Expr;
typedef struct Cond Cond;
typedef struct Iter Iter;
typedef struct Jump Jump;

struct Member {
    Member *next;
    Type *ty;
    int offset;
    int alignment;
};

struct Obj {
    Obj *next;

    Type *ty;
    VarAttrs *attrs;
    char *id;

    int param_count;
    Obj **params;
    BlockItem *block_items;
};

struct VarRef {
    Obj *var;
};

struct FuncInvok {
    Obj *func;

};

struct Expr {
    Type *ty;
    // TODO: what happens with alignments in exprs??
    Expr *l_operand;
    Expr *r_operand;

    VarRef *var_ref; // str literals are put in compiler-generated variables
    FuncInvok *func_invok;
    int64_t val; // numbers and chararcter literal values

    Expr *next;
};

struct Cond {
};

struct Iter {
};

struct Jump {
    JumpType ty;
    char *id; // goto <id>
    Expr *expr; // return <expr>
};

struct BlockItem {
    BlockItemType ty;
    union {
        BlockItem *block_items;
        Expr *expr;
        Cond *cond;
        Iter *iter;
        Jump *jmp;
    };
};

#endif
