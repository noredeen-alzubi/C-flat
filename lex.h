#ifndef LEX_H
#define LEX_H

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

typedef enum {KEYWORD, ID, LITERAL, PUNCTUATOR, CONSTANT} TokenType;

typedef struct Punctuator Punctuator;
struct Punctuator {
    PunctuatorType type;
};

typedef struct Keyword Keyword;
struct Keyword {
    KeywordType type;
};

typedef struct Token Token;
struct Token {
    TokenType type;
    char* text;
    union
    {
        KeywordType keyword_type;
        PunctuatorType punctuator_type;
        ConstantType constant_type;
    };
};

#endif
