#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "lex.h"
#include "token_trie.h"

char* keyword_strings[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_STRING)
};
int keyword_enums[] = {
    FOREACH_KEYWORD_TYPE(GENERATE_ENUM)
};

char* punctuator_strings[] = {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_STRING)
};
int punctuator_enums[] = {
    FOREACH_PUNCTUATOR_TYPE(GENERATE_ENUM)
};

int is_keyword_begin_char(int ch) { return (isalpha(ch) || ch == '_'); }

void skip_whitespace(FILE* fptr) {
    int ch;
    do {
        ch = fgetc(fptr);
    } while (isspace(ch));
    ungetc(ch, fptr);
}

// TODO: make this function nicer
// We assume all C punctuators have size <= 2
Token* attempt_grab_punctuator(FILE* fptr) {
    int ch0 = fgetc(fptr);
    if (ch0 == EOF) {
        ungetc(ch0, fptr);
        return NULL;
    }

    int ch1 = fgetc(fptr);
    for (int i = 0; i < PUNCTUATOR_TYPE_COUNT; ++i) {
        if (ch0 == punctuator_strings[i][0]) {
            Token* t = NULL;
            if (punctuator_strings[i][1] == '\0' || ch1 == punctuator_strings[i][1]) {
                t = malloc(sizeof(Token));
                t->type = PUNCTUATOR;
                t->text = punctuator_strings[i];
                t->punctuator_type = punctuator_enums[i];
                if (punctuator_strings[i][1] == '\0') ungetc(ch1, fptr);
                return t;
            }

            if (punctuator_strings[i][1] == '\0') ungetc(ch1, fptr);
        }
    }
    ungetc(ch1, fptr);
    ungetc(ch0, fptr);
    return NULL;
}

void get_rest_of_ID(FILE* fptr, char** rest) {
    int ch;
    size_t size = 0, len = 0;
    while(isalnum(ch=fgetc(fptr)) || ch == '_') {
        if (len + 1 >= size) {
            size = size * 2 + 1;
            *rest = realloc(*rest, sizeof(char) * size);
        }
        (*rest)[len++] = ch;
    }
    if (*rest != NULL) (*rest)[len] = '\0';
    ungetc(ch, fptr);
}

Token* get_keyword_token(FILE* fptr, TokenTrieNode* keyword_trie_root, char** scanned) {
    int ch;
    size_t size = 0, len = 0;
    TokenTrieNode* curr = keyword_trie_root;
    while(1) {
        ch = fgetc(fptr);

        if (len + 1 >= size) {
            size = size * 2 + 1;
            *scanned = realloc(*scanned, sizeof(char) * size);
        }

        if (ch == EOF || !curr || !curr->children[ch]) {
            ungetc(ch, fptr);
            break;
        }

        (*scanned)[len++] = ch;
        // printf("ch: %c, curr: %c\n", ch, curr->ch);
        curr = curr->children[ch];
    }
    if (*scanned != NULL) (*scanned)[len] = '\0';

    // printf("scanned: %s\n", *scanned);
    // TODO: return deep copy of token?
    return curr ? curr->token : NULL;
}

Token* get_int_or_decimal_constant(FILE* fptr) {
    // assume: ch0 is a digit
    int ch0 = fgetc(fptr);
    int ch1 = fgetc(fptr);

    if (ch0 == '0' && (ch1 == 'x' || ch1 == 'X')) {
        // hex
    }
    else if (ch0 == '0') {
        // octal
    }
    else {
        // decimal
    }

    Token* result = NULL;
    return result;
}

Token* get_char_constant(FILE* fptr) {}

// SO: when scanning IDs, do NOT touch L,u,U with ' after!!!
// actually: first try to grab constants
// if we fail to grab u'<c-char>', then we can grab 'u' as an ID, '\'' as a punct, '<c-char>' as id
Token* attempt_grab_constant(FILE* fptr) {
    Token* result = NULL;
    int ch0 = fgetc(fptr);
    if (ch0 == EOF) {
        ungetc(ch0, fptr);
        return NULL;
    }

    int ch1 = fgetc(fptr);
    ungetc(ch1, fptr);
    ungetc(ch0, fptr);

    if (isdigit(ch0)) get_int_or_decimal_constant(fptr);
    else if (
        (ch0 == '\'') ||
        ((ch0 == 'L' || ch0 == 'u' || ch0 == 'U') && ch1 == '\'')
    ) {
        get_char_constant(fptr);
    }

    return result;
}

Token* get_next_token(FILE* fptr, TokenTrieNode* keyword_trie) {
    Token* result = NULL;
    char* temp;
    int ch;
    do {
        ch = fgetc(fptr);
        ungetc(ch, fptr);
        // printf("-> curr ch: '%c'\n", ch);

        if (isspace(ch)) skip_whitespace(fptr);
        else if ((result = attempt_grab_constant(fptr))) { break; }
        else if ((result = attempt_grab_punctuator(fptr))) { break; }
        else if (is_keyword_begin_char(ch)) {
            char* scanned = NULL;

            Token* keyword_token = get_keyword_token(fptr, keyword_trie, &scanned);
            if (keyword_token) {
                result = keyword_token;
                break;
            }

            // create identifier token
            char* rest_of_id = NULL;
            get_rest_of_ID(fptr, &rest_of_id);

            // TODO: need these?
            int new_size = 0;
            if (scanned) new_size += strlen(scanned);
            if (rest_of_id) new_size += strlen(rest_of_id);

            // printf("rest of ID: %s\n", rest_of_id);
            scanned = realloc(scanned, sizeof(char) * new_size);
            strcat(scanned, rest_of_id);
            Token* token = malloc(sizeof(Token));
            token->type = ID;
            token->text = scanned;
            result = token;
            break;
        }
    } while (ch != EOF);

    return result;
}

int main(int argc, char* argv[])
{
    TokenTrieNode* keyword_trie = build_token_trie(keyword_strings, keyword_enums, KEYWORD_TYPE_COUNT, KEYWORD);
    if (argc < 2) {
        printf("missing argument: file\n");
        return 1;
    }

    FILE* fptr = fopen(argv[1], "r");

    if (fptr == NULL) {
        printf("file can't be opened\n");
        return 1;
    }

    Token* token;
    while ((token = get_next_token(fptr, keyword_trie))) {
        int token_subtype = -1;
        if (token->type == KEYWORD) token_subtype = token->keyword_type;
        else if (token->type == PUNCTUATOR) token_subtype = token->punctuator_type;
        printf("---\nSCANNED: TOKEN(type=%d, subtype=%d, text='%s')\n---\n", token->type, token_subtype, token->text);
    }
}
