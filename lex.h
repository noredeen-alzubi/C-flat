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

#define FOREACH_PUNCTUATOR_TYPE(PUNCTUATOR_TYPE)        \
        PUNCTUATOR_TYPE(LPAREN, "(")          \
        PUNCTUATOR_TYPE(RPAREN, ")")          \
        PUNCTUATOR_TYPE(LBRACE, "{")          \
        PUNCTUATOR_TYPE(RBRACE, "}")          \
        PUNCTUATOR_TYPE(LBRACKET, "[")        \
        PUNCTUATOR_TYPE(RBRACKET, "]")        \
        PUNCTUATOR_TYPE(DOT, ".")             \
        PUNCTUATOR_TYPE(DEREF, "->")          \
        PUNCTUATOR_TYPE(INC, "+=")            \
        PUNCTUATOR_TYPE(DEC, "-=")            \
        PUNCTUATOR_TYPE(ADDR, "&")            \
        PUNCTUATOR_TYPE(STAR, "*")            \
        PUNCTUATOR_TYPE(PLUS, "+")            \
        PUNCTUATOR_TYPE(MINUS, "-")           \
        PUNCTUATOR_TYPE(BNOT, "~")            \
        PUNCTUATOR_TYPE(LNOT, "!")            \
        PUNCTUATOR_TYPE(LNOTEQ, "!=")         \

#define GENERATE_ENUM(ENUM, TEXT) ENUM,
#define GENERATE_STRING(ENUM, TEXT) TEXT,

typedef enum {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
} KeywordType;

typedef enum {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} PunctuatorType;

typedef enum {KEYWORD, ID, LITERAL, PUNCTUATOR} TokenType;

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
    };
};

#endif
