#ifndef CFLAT_H
#define CFLAT_H

#include <stdint.h>
#include <stdlib.h>

#define FOREACH_KEYWORD_TYPE(KEYWORD_TYPE)           \
        KEYWORD_TYPE(AUTO, "auto")              \
        KEYWORD_TYPE(BREAK, "break")            \
        KEYWORD_TYPE(CASE, "case")              \
        KEYWORD_TYPE(CHAR, "char")              \
        KEYWORD_TYPE(INT, "int")                \
        KEYWORD_TYPE(DOUBLE, "double")          \
        KEYWORD_TYPE(FLOAT, "float")            \
        KEYWORD_TYPE(SIGNED, "signed")          \
        KEYWORD_TYPE(UNSIGNED, "unsigned")      \
        KEYWORD_TYPE(VOID, "unsigned")          \
        KEYWORD_TYPE(ENUM, "enum")              \
        KEYWORD_TYPE(CONST, "const")            \
        KEYWORD_TYPE(EXTERN, "extern")          \
        KEYWORD_TYPE(STATIC, "static")          \
        KEYWORD_TYPE(CONTINUE, "continue")      \
        KEYWORD_TYPE(DEFAULT, "default")        \
        KEYWORD_TYPE(DO, "do")                  \
        KEYWORD_TYPE(ELSE, "else")              \
        KEYWORD_TYPE(IF, "if")                  \

#define KEYWORD_TYPE_COUNT 19

// IMPORTANT: multi-char punctuators come first in the list
#define FOREACH_PUNCTUATOR_TYPE(PUNCTUATOR_TYPE)        \
        PUNCTUATOR_TYPE(DEREF, "->")          \
        PUNCTUATOR_TYPE(LNOTEQ, "!=")         \
        PUNCTUATOR_TYPE(INC, "+=")            \
        PUNCTUATOR_TYPE(DEC, "-=")            \
        PUNCTUATOR_TYPE(INC_ONE, "++")            \
        PUNCTUATOR_TYPE(DEC_ONE, "--")            \
        PUNCTUATOR_TYPE(SEMICOLON, ";")       \
        PUNCTUATOR_TYPE(LPAREN, "(")          \
        PUNCTUATOR_TYPE(RPAREN, ")")          \
        PUNCTUATOR_TYPE(LBRACE, "{")          \
        PUNCTUATOR_TYPE(RBRACE, "}")          \
        PUNCTUATOR_TYPE(LBRACKET, "[")        \
        PUNCTUATOR_TYPE(RBRACKET, "]")        \
        PUNCTUATOR_TYPE(DOT, ".")             \
        PUNCTUATOR_TYPE(ADDR, "&")            \
        PUNCTUATOR_TYPE(STAR, "*")            \
        PUNCTUATOR_TYPE(PLUS, "+")            \
        PUNCTUATOR_TYPE(EQ, "=")            \
        PUNCTUATOR_TYPE(MINUS, "-")           \
        PUNCTUATOR_TYPE(BNOT, "~")            \
        PUNCTUATOR_TYPE(LNOT, "!")            \

#define PUNCTUATOR_TYPE_COUNT 18

#define GENERATE_ENUM(ENUM, TEXT) ENUM,
#define GENERATE_STRING(ENUM, TEXT) TEXT,

typedef enum {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
} KeywordType;

typedef enum {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} PunctuatorType;

typedef enum {
    DECIMAL_INT, DECIMAL_FLOAT, HEX_INT, HEX_FLOAT, OCTAL_INT, ENUMERATION, CHARAC
} ConstantType;

typedef enum {KEYWORD, ID, PUNCTUATOR, CONSTANT, STRING_LIT} TokenType;

typedef struct string dstring;
struct string {
    char* str;
    size_t size, len;
};

typedef struct Token Token;
struct Token {
    TokenType type;
    dstring text;
    int64_t i_value;
    long double f_value;
    dstring s_value;
    union
    {
        KeywordType keyword_type;
        PunctuatorType punctuator_type;
        ConstantType constant_type;
    };
};

#define ALPHABET_SIZE 128

typedef struct TokenTrieNode TokenTrieNode;
struct TokenTrieNode {
    char ch;
    TokenTrieNode* children[ALPHABET_SIZE];
    Token* token;
};

TokenTrieNode* build_token_trie(char** strings, int* subtypes, size_t count, TokenType type);

void dstring_append(dstring* dstr, char c);
char dstring_at(dstring* dstr, int i);
char dstring_set(dstring* dstr, int i, char c);
void dstring_initialize(dstring* dstr);
void dstring_initialize_str(dstring* dstr, char* str, int n);
void dstring_cat(dstring* dest, dstring* src);
void dstring_reserve(dstring* dstr, size_t len);
void dstring_free(dstring dstr);

#endif
