#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
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

int fpeek(FILE* fp) {
    const int c = getc(fp);
    return c == EOF ? EOF : ungetc(c, fp);
}

void skip_whitespace(FILE* fptr) {
    int ch;
    do {
        ch = fgetc(fptr);
    } while (isspace(ch));
    ungetc(ch, fptr);
}

// TODO: make this function nicer
// We assume all C punctuators have size <= 2
Token* scan_punctuator(FILE* fptr) {
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

        if (ch == EOF || !curr || !curr->children[ch]) {
            ungetc(ch, fptr);
            break;
        }

        if (len + 1 >= size) {
            size = size * 2 + 1;
            *scanned = realloc(*scanned, sizeof(char) * size);
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

Token* get_numerical_constant(FILE* fptr) {
    Token* result = NULL;

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

    return result;
}

void put_back_str(FILE* fptr, char scanned[], size_t len) {
    for (int i = len - 1; i >= 0; i--) ungetc(scanned[i], fptr);
}

// For now, wide characters are not supported
Token* get_escape_sequence(char scanned[], char* raw_ch_start, size_t raw_ch_len, char size_modifier) {
    // scanned = L'\ ... ' OR '\ ...'
    Token* result = NULL;
    char* number = strndup(raw_ch_start, raw_ch_len);

    if (*raw_ch_start == 'x') {
        // hex
        long int l_num = strtol(number, NULL, 16);
        if (errno == ERANGE || l_num > CHAR_MAX || l_num < CHAR_MIN) {
            fprintf(stderr, "err: hex char out of range");
            exit(1);
        }
        // TODO create token
    } else if (isdigit(*raw_ch_start)) {
        // octal
        long int l_num = strtol(number, NULL, 8);
        if (errno == ERANGE || l_num > CHAR_MAX || l_num < CHAR_MIN) {
            fprintf(stderr, "err: octal char out of range");
            exit(1);
        }
        // TODO create token
    } else if (*raw_ch_start == 'u') {
        // universal character name
        fprintf(stderr, "unimplemented: universal character name\n");
        exit(2);
    } else {
        switch (*raw_ch_start) {
            case '"':  case '?':  case '\\':
            case 'a':  case 'b':  case 'n':
            case 'r':  case 't':  case 'v':  case 'f':
                // stuff
                break;
            default:
                // invalid escape sequence
                fprintf(stderr, "err: invalid escape sequence\n");
                exit(1);
        }
    }

    return result;
}

Token* get_non_escaped_charac(char scanned[], size_t len) {
    char ch = scanned[len - 3];
    if (ch == '\'' || ch == '\\' || ch == '\n') {
        return NULL;
    }
    size_t charac_bytes = scanned[0] == '\'' ? len - 1 : len - 2;
    if (charac_bytes > 4) {
        // warn: charac constant too long for its type
    }
    // do work
    Token* result = malloc(sizeof(Token));
    result->type = CONSTANT;
    result->constant_type = CHARAC;
    result->text = scanned;

    return result;
}

Token* get_charac_constant(FILE* fptr) {
    // assume ch0 = l | u | u | ', ch1
    // only looks for closing ' until \n or eof (or \r?)
    // for numerical escape sequences, range is 0-255
    // hex: \xff
    // octal: \377
    int ch;
    Token* result = NULL;

    char* scanned = malloc(8 * sizeof(char));
    size_t len = 0;

    // Assume unescaped and no size modifier
    char* raw_ch_start = scanned + 1;
    char* raw_ch_end;
    char size_modifier = '\0';
    int escaped = 0;

    ch = fgetc(fptr);
    // TODO: check EOF
    scanned[len++] = ch;
    if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'u') {
        size_modifier = ch;
        raw_ch_start++;

        ch = fgetc(fptr); // read the opening '
        scanned[len++] = ch;
    }

    ch = fgetc(fptr);
    // TODO: check EOF
    scanned[len++] = ch;
    if (ch == '\\') {
        raw_ch_start++;
    }

    // look for closing '
    while ((ch = fgetc(fptr)) != '\'') {
        if (ch == EOF || ch == '\n') {
            fprintf(stderr, "err: missing closing '\n");
            exit(1);
        }
        scanned[len++] = ch;
    }
    raw_ch_end = scanned + len - 1;
    scanned[len++] = '\'';
    scanned[len++] = '\0';

    if (scanned[1] == '\\' && len > 4) {
        result = get_escape_sequence(scanned, raw_ch_start, raw_ch_end-raw_ch_start, size_modifier);
    } else {
        result = get_non_escaped_charac(scanned, len);
    }

    return result;
}

// NOTE: if we fail to grab u'<c-char>', then we can grab 'u' as an ID, '\'' as a punct, '<c-char>' as id
Token* scan_constant(FILE* fptr) {
    Token* result = NULL;

    int ch0 = fpeek(fptr);
    if (ch0 == EOF) return NULL;
    int ch1 = fpeek(fptr);

    if (isdigit(ch0)) get_numerical_constant(fptr);
    else if (
        (ch0 == '\'') ||
        ((ch0 == 'L' || ch0 == 'u' || ch0 == 'U') && ch1 == '\'')
    ) {
        result = get_charac_constant(fptr);
    }

    return result;
}

Token* scan_keyword_or_id(FILE* fptr, TokenTrieNode* keyword_trie) {
    char ch = fpeek(fptr);
    char* scanned = NULL;

    if (!isalpha(ch) && ch != '_') {
        return NULL;
    }

    Token* keyword_token = get_keyword_token(fptr, keyword_trie, &scanned);
    if (keyword_token) return keyword_token;

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
    return token;
}

Token* get_next_token(FILE* fptr, TokenTrieNode* keyword_trie) {
    Token* result = NULL;
    char* temp;
    int ch;
    do {
        ch = fpeek(fptr);
        // printf("-> curr ch: '%c'\n", ch);

        if (isspace(ch)) skip_whitespace(fptr);
        else if ((result = scan_constant(fptr))) { break; }
        else if ((result = scan_punctuator(fptr))) { break; }
        else if ((result = scan_keyword_or_id(fptr, keyword_trie))) { break; }
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
