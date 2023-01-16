#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

typedef enum KeywordTypeEnum {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
} KeywordType;

typedef enum PunctuatorTypeEnum {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
} PunctuatorType;

typedef enum TokenTypeEnum {KEYWORD, ID, LITERAL, PUNCTUATOR} TokenType;

typedef struct PunctuatorStruct {
    PunctuatorType type;
} Punctuator;

typedef struct KeywordStruct {
    KeywordType type;
} Keyword;

typedef struct TokenStruct {
    TokenType type;
    char* text;
    union
    {
        Keyword keyword;
        Punctuator punctuator;
    };
} Token;

static const char* keyword_strings[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_STRING)
};
static const int keyword_enums[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
};

void skip_whitespace(FILE* fptr) {
    int ch;
    do {
        ch = fgetc(fptr);
    } while (isspace(ch));
    ungetc(ch, fptr);
}

char* get_id_or_keyword(FILE* fptr) {
    char* str = NULL;
    size_t size = 0, len = 0;
    int ch;
    while (isalpha((ch=fgetc(fptr))) || ch == '_') {
        if (len + 1 >= size) {
            size = size * 2 + 1;
            str = realloc(str, sizeof(char)*size);
        }
        str[len++] = ch;
    }
    if (str != NULL) str[len] = '\0';
    ungetc(ch, fptr);
    return str;
}

int is_part_of_keyword(int* cptr, FILE* fptr) {
    return isalpha((*cptr=fgetc(fptr))) || *cptr == '_';
}
char* get_token_value(FILE* fptr, int (*cond)(int*,FILE*)) {
    char* str = NULL;
    size_t size = 0, len = 0;
    int ch;
    while ((*cond)(&ch,fptr)) {
        if (len + 1 >= size) {
            size = size * 2 + 1;
            str = realloc(str, sizeof(char)*size);
        }
        str[len++] = ch;
    }
    if (str != NULL) str[len] = '\0';
    ungetc(ch, fptr);
    return str;
}

int find_string(const char* strings[], size_t size, char* string) {
    for (int i = 0; i < size; ++i) {
        if (strcmp(strings[i], string) == 0) return i;
    }
    return -1;
}

Token* get_next_token(FILE* fptr) {
    int ch;
    do {
        ch = fgetc(fptr);
        ungetc(ch, fptr);

        if (isspace(ch)) skip_whitespace(fptr);
        else if (isalpha(ch) || ch == '_') {
            char* value = get_token_value(fptr, is_part_of_keyword);
            int index = find_string(keyword_strings, KEYWORD_TYPE_COUNT, value);
            if (index > -1) {
                // create keyword token
                Token* token = malloc(sizeof(Token));
                token->type = KEYWORD;
                token->text = value;
                Keyword keyword_obj = { keyword_enums[index] };
                token->keyword = keyword_obj;
                return token;
            } else {
                // create identifier token
                Token* token = malloc(sizeof(Token));
                token->type = ID;
                token->text = value;
                return token;
            }
        }
    } while (ch != EOF);
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        printf("missing argument: file \n");
        return 1;
    }

    FILE* fptr = fopen(argv[1], "r");

    if (fptr == NULL) {
        printf("file can't be opened \n");
        return 1;
    }

    Token* token;
    while ((token = get_next_token(fptr))) {
        int token_subtype = -1;
        if (token->type == KEYWORD) token_subtype = token->keyword.type;
        else if (token->type == PUNCTUATOR) token_subtype = token->punctuator.type;
        printf("TOKEN(type=%d, subtype=%d, text='%s')\n", token->type, token_subtype, token->text);
    }
}
