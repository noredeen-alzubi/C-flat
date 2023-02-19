#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/types.h>
#include "cflat.h"

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

int fpeek(FILE* fp)
{
    const int c = getc(fp);
    return c == EOF ? EOF : ungetc(c, fp);
}

void skip_whitespace(FILE* fptr)
{
    int ch;
    do {
        ch = fgetc(fptr);
    } while (isspace(ch));
    ungetc(ch, fptr);
}

// TODO: make this function nicer
// We assume all C punctuators have size <= 2
Token* scan_punctuator(FILE* fptr)
{
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
                dstring_initialize_str(&t->text, punctuator_strings[i]);
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

void get_rest_of_id(FILE* fptr, dstring* rest)
{
    int ch;
    while(isalnum(ch=fgetc(fptr)) || ch == '_') {
        dstring_append(rest, ch);
    }
    ungetc(ch, fptr);
}

Token* get_keyword_token(
    FILE* fptr,
    TokenTrieNode* keyword_trie_root,
    dstring* scanned
)
{
    int ch;
    TokenTrieNode* curr = keyword_trie_root;
    while(1) {
        ch = fgetc(fptr);

        if (ch == EOF || !curr || !curr->children[ch]) {
            ungetc(ch, fptr);
            break;
        }

        dstring_append(scanned, ch);
        // printf("ch: %c, curr: %c\n", ch, curr->ch);
        curr = curr->children[ch];
    }

    // printf("scanned: %s\n", *scanned);
    // TODO: return deep copy of token?
    return curr ? curr->token : NULL;
}

bool is_digit(char c, int base) {
    return c >= '0' && c <= '9' ||
        (base > 10 && c >= 'A' && c <= ('A'+base-10));
}

// TODO: floating point? later
Token* scan_numerical_constant(FILE* fptr)
{
    Token* result = NULL;

    // assume: ch0 is a digit
    int ch0 = fgetc(fptr);
    int ch1 = fgetc(fptr);
    int base;
    ConstantType type;

    dstring scanned;
    dstring_initialize(&scanned);

    // we dont ungetc() ch0 and ch1 in this case so we discard the 0x prefix
    if (ch0 == '0' && (ch1 == 'x' || ch1 == 'X')) {
        base = 16;
        type = HEX_INT;
        dstring_append(&scanned, ch0);
        dstring_append(&scanned, ch1);
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

    while(is_digit((ch0 = fgetc(fptr)), base)) { dstring_append(&scanned, ch0); }
    ungetc(ch0, fptr);

    if (type == HEX_INT && scanned.len < 3) {
        fprintf(stderr, "err: missing hex digits from integer literal\n");
        exit(1);
    }

    int64_t l_num = strtoul(type == HEX_INT ? scanned.str+2 : scanned.str, NULL, base);
    if (errno == ERANGE || l_num > ULONG_MAX) {
        fprintf(stderr, "err: integer literal is too big\n");
        exit(1);
    }

    result = malloc(sizeof(Token));
    result->type = CONSTANT;
    result->constant_type = type;
    result->i_value = l_num;
    // TODO: does this work???
    memcpy(&result->text, &scanned, sizeof(dstring));

    return result;
}

void put_back_str(FILE* fptr, char scanned[], size_t len)
{
    for (int i = len - 1; i >= 0; i--) ungetc(scanned[i], fptr);
}

// For now, wide characters are not supported
Token* scan_escape_sequence(
   // char scanned[],
   dstring* scanned,
   char* raw_ch_start,
   size_t raw_ch_len,
   char size_modifier
)
{
    // scanned = L'\ ... ' OR '\ ...'
    Token* result = NULL;
    char* number = strndup(raw_ch_start, raw_ch_len);

    if (number[0] == 'x') {
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
        memcpy(&result->text, &scanned, sizeof(dstring));
    } else if (isdigit(number[0])) {
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
    } else if (number[0] == 'u') {
        // universal character name
        fprintf(stderr, "unimplemented: universal character name\n");
        exit(2);
    } else {
        switch (number[0]) {
            case '"':  case '?':  case '\\':
            case 'a':  case 'b':  case 'n':
            case 'r':  case 't':  case 'v':  case 'f':
                result = malloc(sizeof(Token));
                result->type = CONSTANT;
                result->constant_type = CHARAC;
                result->i_value = *raw_ch_start;
                break;
            default:
                fprintf(stderr, "err: invalid escape sequence\n");
                exit(1);
        }
    }

    return result;
}

Token* scan_non_escaped_charac(dstring* scanned/*char scanned[]*/)
{
    // '...' OR u'...'
    size_t charac_len = scanned->str[0] == '\'' ? scanned->len-2 : scanned->len-3;
    if (charac_len > 1) {
        fprintf(stderr, "err: multi-character character literals not supported\n");
        exit(1);
    }

    // TODO: need this?
    char ch = dstring_at(scanned, scanned->len-2);
    if (ch == '\'' || ch == '\\' || ch == '\n') {
        return NULL;
    }

    Token* result = malloc(sizeof(Token));
    result->type = CONSTANT;
    result->constant_type = CHARAC;
    memcpy(&result->text, &scanned, sizeof(dstring));

    return result;
}

Token* scan_charac_constant(FILE* fptr)
{
    // only looks for closing ' until \n or eof (or \r?)
    // for numerical escape sequences, range is 0-255
    // hex: \xff
    // octal: \377
    int ch;
    Token* result = NULL;

    // char* scanned = malloc(8 * sizeof(char));
    // size_t len = 0;
    dstring scanned;
    dstring_reserve(&scanned, 8);

    // Assume unescaped and no size modifier
    char* raw_ch_start = scanned.str + 1;
    char* raw_ch_end;
    char size_modifier = '\0';
    bool escaped = false;

    ch = fgetc(fptr);
    if (ch == EOF) return NULL;
    dstring_append(&scanned, ch);
    // scanned[len++] = ch;
    if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'u') {
        size_modifier = ch;
        raw_ch_start++;

        ch = fgetc(fptr); // read the opening '
        dstring_append(&scanned, ch);
        // scanned[len++] = ch;
    }

    ch = fgetc(fptr);
    dstring_append(&scanned, ch);
    // scanned[len++] = ch;
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
        dstring_append(&scanned, ch);
        // scanned[len++] = ch;
    }
    raw_ch_end = scanned.str + scanned.len - 1;
    dstring_append(&scanned, '\'');
    // scanned[len++] = '\'';
    // scanned[len++] = '\0';

    if (escaped) {
        result = scan_escape_sequence(&scanned, raw_ch_start, raw_ch_end-raw_ch_start+1, size_modifier);
    } else {
        result = scan_non_escaped_charac(&scanned);
    }

    return result;
}

// NOTE: if we fail to grab u'<c-char>', then we can grab 'u' as an ID, '\'' as a punct, '<c-char>' as id
Token* scan_constant(FILE* fptr)
{
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

Token* scan_string_literal(FILE* fptr)
{
}

Token* scan_keyword_or_id(FILE* fptr, TokenTrieNode* keyword_trie)
{
    char ch = fpeek(fptr);
    dstring scanned;
    dstring_initialize(&scanned);

    if (!isalpha(ch) && ch != '_') {
        return NULL;
    }

    Token* keyword_token = get_keyword_token(fptr, keyword_trie, &scanned);
    if (keyword_token) return keyword_token;

    dstring rest_of_id;
    dstring_initialize(&rest_of_id);
    get_rest_of_id(fptr, &rest_of_id);

    // printf("scanned: %s\n", scanned);
    // printf("rest of ID: %s\n", rest_of_id);
    // printf("len(rest of ID): %i\n", (unsigned int) strlen(rest_of_id));
    // printf("combined size: %i\n", new_size);

    dstring_cat(&scanned, &rest_of_id);
    Token* token = malloc(sizeof(Token));
    token->type = ID;
    memcpy(&token->text, &scanned, sizeof(dstring));
    return token;
}

Token* get_next_token(FILE* fptr, TokenTrieNode* keyword_trie)
{
    Token* result = NULL;
    char* temp;
    int ch;
    do {
        ch = fpeek(fptr);

        if (isspace(ch)) skip_whitespace(fptr);
        else if ((result = scan_constant(fptr))) { break; }
        else if ((result = scan_string_literal(fptr))) { break; }
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
                token->type, token_subtype, token->text.str, token->i_value);
    }
}
