#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "token_trie.h"

char* keyword_strings[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_STRING)
};
int keyword_enums[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
};

void skip_whitespace(FILE* fptr) {
    int ch;
    do {
        ch = fgetc(fptr);
    } while (isspace(ch));
    ungetc(ch, fptr);
}

char* get_punctuator(FILE* fptr) {
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

// char* get_id_or_keyword(FILE* fptr) {
//     char* str = NULL;
//     size_t size = 0, len = 0;
//     int ch;
//     while (isalpha((ch=fgetc(fptr))) || ch == '_') {
//         if (len + 1 >= size) {
//             size = size * 2 + 1;
//             str = realloc(str, sizeof(char)*size);
//         }
//         str[len++] = ch;
//     }
//     if (str != NULL) str[len] = '\0';
//     ungetc(ch, fptr);
//     return str;
// }

int is_part_of_keyword(int* cptr, FILE* fptr) {
    return isalpha((*cptr=fgetc(fptr))) || *cptr == '_';
}

// char* get_token_value(FILE* fptr, int (*cond)(int*,FILE*)) {
//     char* str = NULL;
//     size_t size = 0, len = 0;
//     int ch;
//     while ((*cond)(&ch,fptr)) {
//         if (len + 1 >= size) {
//             size = size * 2 + 1;
//             str = realloc(str, sizeof(char)*size);
//         }
//         str[len++] = ch;
//     }
//     if (str != NULL) str[len] = '\0';
//     ungetc(ch, fptr);
//     return str;
// }

void get_rest_of_ID(FILE* fptr, char* rest) {
    int ch;
    size_t size = 0, len = 0;
    while(isalnum(ch=fgetc(fptr)) || ch == '_') {
        if (len + 1 >= size) {
            size = size * 2 + 1;
            rest = realloc(rest, sizeof(char) * size);
        }
        rest[len++] = ch;
    }
    if (rest != NULL) rest[len] = '\0';
    ungetc(ch, fptr);
}

// int find_string(const char* strings[], size_t size, char* string) {
//     for (int i = 0; i < size; ++i) {
//         if (strcmp(strings[i], string) == 0) return i;
//     }
//     return -1;
// }

Token* get_keyword_token(FILE* fptr, TokenTrieNode* keyword_trie_root, char* scanned) {
    int ch;
    size_t size = 0, len = 0;
    TokenTrieNode* curr = keyword_trie_root;
    while(1) {
        ch = fgetc(fptr);
        if (ch == EOF || !curr->children[ch]) {
            ungetc(ch, fptr);
            break;
        }

        if (len + 1 >= size) {
            size = size * 2 + 1;
            scanned = realloc(scanned, sizeof(char) * size);
        }
        scanned[len++] = ch;
        curr = curr->children[ch];
    }
    if (scanned != NULL) scanned[len] = '\0';

    return curr->token;
}

Token* get_next_token(FILE* fptr, TokenTrieNode* keyword_trie) {
    Token* result = NULL;
    int ch;
    do {
        ch = fgetc(fptr);
        ungetc(ch, fptr);

        if (isspace(ch)) skip_whitespace(fptr);
        else if (isalpha(ch) || ch == '_') {
            char* scanned;
            Token* keyword_token = get_keyword_token(fptr, keyword_trie, scanned);
            if (!keyword_token) {
                // create identifier token
                char* rest_of_id;
                get_rest_of_ID(fptr, rest_of_id);
                scanned = realloc(scanned, sizeof(char) * (strlen(rest_of_id)+strlen(scanned)+1));
                strcat(scanned, rest_of_id);
                Token* token = malloc(sizeof(Token));
                token->type = ID;
                token->text = scanned;
                result = token;
                break;
            } else {
                result = keyword_token;
                break;
            }
        }
    } while (ch != EOF);

    return result;
}

int main(int argc, char* argv[])
{
    // TODO
    TokenTrieNode* keyword_trie = build_token_trie(keyword_strings, keyword_enums, KEYWORD_TYPE_COUNT, KEYWORD);
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
    while ((token = get_next_token(fptr, keyword_trie))) {
        int token_subtype = -1;
        if (token->type == KEYWORD) token_subtype = token->keyword_type;
        else if (token->type == PUNCTUATOR) token_subtype = token->punctuator_type;
        printf("TOKEN(type=%d, subtype=%d, text='%s')\n", token->type, token_subtype, token->text);
    }
}
