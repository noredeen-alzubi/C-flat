#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/types.h>
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

// TODO: this functions is shameful. needs to go
void get_rest_of_ID(FILE* fptr, char** rest) {
    size_t size = 1, len = 0;
    *rest = malloc(size);

    int ch;
    while(isalnum(ch=fgetc(fptr)) || ch == '_') {
        if (len + 1 >= size) {
            size = size * 2 + 1;
            *rest = realloc(*rest, sizeof(char) * size);
        }
        (*rest)[len++] = ch;
    }
    ungetc(ch, fptr);

    // ew
    if (len + 1 >= size) {
        size = size * 2 + 1;
        *rest = realloc(*rest, sizeof(char) * size);
    }
    (*rest)[len++] = '\0';

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

// TODO: floating point?
Token* scan_numerical_constant(FILE* fptr) {
    Token* result = NULL;

    // assume: ch0 is a digit
    int ch0 = fgetc(fptr);
    int ch1 = fgetc(fptr);
    int base;
    ConstantType type;

    char* scanned = malloc(3);
    size_t size = 3, len = 0;

    // we dont ungetc() ch0 and ch1 in this case so we discard the 0x prefix
    if (ch0 == '0' && (ch1 == 'x' || ch1 == 'X')) {
        base = 16;
        type = HEX_INT;
        scanned[0] = ch0;
        scanned[1] = ch1;
        len = 2;
    }
    else {
        if (ch0 == '0') {
            base = 8;
            type = OCTAL_INT;
        }
        else {
            base = 10;
            type = DECIMAL_INT;
        }
        ungetc(ch1, fptr);
        ungetc(ch0, fptr);
    }

    while((ch0 = fgetc(fptr)) >= '0' && ch0 <= '9' ||
            (base > 10 && ch0 >= 'A' && ch0 <= ('A'+base-10))) {
        if (len + 2 >= size) {
            size = size * 2 + 1;
            scanned = realloc(scanned, sizeof(char) * size);
        }
        scanned[len++] = ch0;
    }
    scanned[len++] = '\0';
    ungetc(ch0, fptr);

    if (type == HEX_INT && len < 4) {
        fprintf(stderr, "err: missing hex digits from integer literal\n");
        exit(1);
    }

    int64_t l_num = strtoul(type == HEX_INT ? scanned+2 : scanned, NULL, base);
    if (errno == ERANGE || l_num > ULONG_MAX) {
        fprintf(stderr, "err: integer literal is too big\n");
        exit(1);
    }

    result = malloc(sizeof(Token));
    result->type = CONSTANT;
    result->constant_type = type;
    result->i_value = l_num;
    result->text = scanned;

    return result;
}

void put_back_str(FILE* fptr, char scanned[], size_t len) {
    for (int i = len - 1; i >= 0; i--) ungetc(scanned[i], fptr);
}

// For now, wide characters are not supported
Token* scan_escape_sequence(char scanned[], char* raw_ch_start, size_t raw_ch_len, char size_modifier) {
    // scanned = L'\ ... ' OR '\ ...'
    Token* result = NULL;
    char* number = strndup(raw_ch_start, raw_ch_len);

    if (*raw_ch_start == 'x') {
        // hex
        long int l_num = strtol(number+1, NULL, 16);
        if (errno == ERANGE || l_num > UCHAR_MAX) {
            fprintf(stderr, "err: hex char out of range\n");
            exit(1);
        }
        result = malloc(sizeof(Token));
        result->type = CONSTANT;
        result->constant_type = CHARAC;
        result->i_value = l_num;
        result->text = scanned;
    } else if (isdigit(*raw_ch_start)) {
        // octal
        long int l_num = strtol(number, NULL, 8);
        if (errno == ERANGE || l_num > UCHAR_MAX) {
            fprintf(stderr, "err: octal char out of range\n");
            exit(1);
        }
        result = malloc(sizeof(Token));
        result->type = CONSTANT;
        result->constant_type = CHARAC;
        result->i_value = l_num;
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
                result = malloc(sizeof(Token));
                result->type = CONSTANT;
                result->constant_type = CHARAC;
                result->i_value = *raw_ch_start;
                break;
            default:
                // invalid escape sequence
                fprintf(stderr, "err: invalid escape sequence\n");
                exit(1);
        }
    }

    return result;
}

Token* scan_non_escaped_charac(char scanned[], size_t len) {
    char ch = scanned[len - 3];
    if (ch == '\'' || ch == '\\' || ch == '\n') {
        return NULL;
    }
    size_t charac_len = scanned[0] == '\'' ? len - 1 : len - 2;
    // TODO: huh? why this?
    if (charac_len > 4) {
        fprintf(stderr, "err: multi-character character constants not supported\n");
        exit(1);
    }
    Token* result = malloc(sizeof(Token));
    result->type = CONSTANT;
    result->constant_type = CHARAC;
    result->text = scanned;

    return result;
}

Token* scan_charac_constant(FILE* fptr) {
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
    bool escaped = false;

    ch = fgetc(fptr);
    if (ch == EOF) return NULL;
    scanned[len++] = ch;
    if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'u') {
        size_modifier = ch;
        raw_ch_start++;

        ch = fgetc(fptr); // read the opening '
        scanned[len++] = ch;
    }

    ch = fgetc(fptr);
    scanned[len++] = ch;
    if (ch == '\\') {
        raw_ch_start++;
        escaped = true;
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

    if (escaped) {
        result = scan_escape_sequence(scanned, raw_ch_start, raw_ch_end-raw_ch_start+1, size_modifier);
    } else {
        result = scan_non_escaped_charac(scanned, len);
    }

    return result;
}

// NOTE: if we fail to grab u'<c-char>', then we can grab 'u' as an ID, '\'' as a punct, '<c-char>' as id
Token* scan_constant(FILE* fptr) {
    Token* result = NULL;

    int ch0 = fgetc(fptr);
    if (ch0 == EOF) {
        ungetc(ch0, fptr);
        return NULL;
    }
    int ch1 = fpeek(fptr);
    ungetc(ch0, fptr);

    if (isdigit(ch0)) result = scan_numerical_constant(fptr);
    else if (
        (ch0 == '\'') ||
        ((ch0 == 'L' || ch0 == 'u' || ch0 == 'U') && ch1 == '\'')
    ) {
        result = scan_charac_constant(fptr);
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
    int new_size = 1;
    if (scanned) new_size += strlen(scanned);
    if (rest_of_id) new_size += strlen(rest_of_id);

    // printf("scanned: %s\n", scanned);
    // printf("rest of ID: %s\n", rest_of_id);
    // printf("len(rest of ID): %i\n", (unsigned int) strlen(rest_of_id));
    // printf("combined size: %i\n", new_size);
    scanned = realloc(scanned, sizeof(char)*new_size);
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

        if (isspace(ch)) skip_whitespace(fptr);
        else if ((result = scan_constant(fptr))) { break; }
        else if ((result = scan_punctuator(fptr))) { break; }
        else if ((result = scan_keyword_or_id(fptr, keyword_trie))) { break; }
        else if (ch == EOF) { break; }
        else {
            fprintf(stderr, "err: can't recognize token\n");
            exit(1);
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
        int token_subtype = token->type == KEYWORD ?
            token->keyword_type : token->punctuator_type;
        printf("---\nSCANNED: TOKEN(type=%d, subtype=%d,\
                text=\"%s\", i_value=%li)\
                \n---\n",
                token->type, token_subtype, token->text, token->i_value);
    }
}
