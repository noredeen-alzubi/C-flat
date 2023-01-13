#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

enum KeywordTypeEnum {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
} typedef KeywordType;

enum PunctuatorTypeEnum {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} typedef PunctuatorType;

enum TokenTypeEnum {KEYWORD, ID, LITERAL, PUNCTUATOR} typedef TokenType;

struct PunctuatorStruct {
    PunctuatorType type;
} typedef Punctuator;

struct KeywordStruct {
    KeywordType type;
} typedef Keyword;

struct TokenStruct {
    TokenType type;
    char* text;
    union
    {
        Keyword keyword;
        Punctuator punctuator;
    };
} typedef Token;

Token get_next_token(FILE* ptr) {
    int ch;
    do {
        ch = fgetc(ptr);
        // stuff
    } while (ch != EOF);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("missing argument: file \n");
        return 1;
    }

    FILE* ptr = fopen(argv[1], "r");

    if (ptr == NULL) {
        printf("file can't be opened \n");
        return 1;
    }

    char ch;
    do {
        ch = fgetc(ptr);
        // stuff
    } while (ch != EOF);
}
