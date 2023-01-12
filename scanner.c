#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum KeywordTypeEnum
{
    AUTO,
    BREAK,
    CASE,
    CHAR,
    CONST,
    CONTINUE,
    DEFAULT,
    DO,
    DOUBLE,
    ELSE,
    ENUM,
    EXTERN,
    FLOAT,
    FOR,
    GOTO,
    IF,
    INLINE,
    INT,
    LONG,
    REGISTER,
    RESTRICT,
    RETURN,
    SHORT,
    SIGNED,
    SIZEOF,
    STATIC,
    STRUCT,
    SWITCH,
    TYPEDEF,
    UNION,
    UNSIGNED,
    VOID,
    VOLATILE,
    WHILE
} typedef KeywordType;

enum PunctuatorType
{
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    LBRACKET, RBRACKET,
    DOT,
    DEREF,
    INC,
    DEC,
    ADDR,
    STAR,
    PLUS,
    MINUS,
    BNOT,
    LNOT,
    SLASH,
    PCT,
    BLSHIFT,
    BRSHIFT,
    LT,
    GT,
    LE,
    GE,
    EQ,
    NE,
    LAND

} typedef Punctuator;

struct KeywordStruct {
    KeywordType type;
} typedef Keyword;

enum TokenTypeEnum {KEYWORD, ID, CONSTANT, PUNCTUATOR} typedef TokenType;
struct TokenStruct {
    TokenType type;
    char* value;
    union
    {
        Keyword keyword;
    };
} typedef Token;

Token get_token();

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("missing argument: file \n");
        return 1;
    }

    FILE* ptr = fopen(argv[1], "r");
    char ch;

    if (ptr == NULL) {
        printf("file can't be opened \n");
        return 1;
    }

    do {
        ch = fgetc(ptr);
        // stuff
    } while (ch != EOF);
}
