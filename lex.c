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
                dstring_initialize_str(&t->text, punctuator_strings[i], -1);
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
        curr = curr->children[ch];
    }

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

    uint64_t l_num = strtoul(type == HEX_INT ? scanned.str+2 : scanned.str, NULL, base);
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

// TODO: what if just "\" or "\x"
// returns index in dstr right after end of escape sequence
int get_escape_sequence(dstring* dstr, int64_t* value, int* base)
{
    assert(dstring_at(dstr, 0) == '\\');

    if (dstr->len <= 1) {
        fprintf(stderr, "err: invalid escape sequence\n");
        exit(1);
    }

    *base = -1;
    char first_char = dstring_at(dstr, 1);
    if (first_char == 'x') {
        // hex
        *base = 16;
    } else if (first_char == 'u') {
        // universal character name
        fprintf(stderr, "unimplemented: universal character name\n");
        exit(2);
    } else if (isdigit(first_char)) {
        // octal
        *base = 8;
    } else {
        // simple escape sequence
        switch (first_char) {
            case '"':
                *value = '\"';
                break;
            case '?':
                *value = '\?';
                break;
            case '\\':
                *value = '\\';
                break;
            case 'a':
                *value = '\a';
                break;
            case 'b':
                *value = '\b';
                break;
            case 'n':
                *value = '\n';
                break;
            case 'r':
                *value = '\r';
                break;
            case 't':
                *value = '\t';
                break;
            case 'v':
                *value = '\v';
                break;
            case 'f':
                *value = '\f';
                break;
            default:
                fprintf(stderr, "err: invalid escape sequence\n");
                exit(1);
        }

        return 2;
    }

    int i = 2;
    while (i < dstr->len && is_digit(dstring_at(dstr, i), *base)) { ++i; }
    if (i == 2) {
        fprintf(stderr, "err: missing digits in char escape sequence\n");
        exit(1);
    }

    char* number = strndup(dstr->str+2, i-2);
    uint64_t l_num = strtoul(number, NULL, *base);
    if (errno == ERANGE || l_num > UCHAR_MAX) {
        fprintf(stderr, "err: char escape sequence is too big\n");
        exit(1);
    }
    *value = l_num;

    return i;
}

// For now, wide characters are not supported
Token* scan_char_escape_sequence(
   dstring* scanned,
   dstring* raw_dstr,
   char size_modifier
)
{
    // scanned = L'\ ... ' OR '\ ...'
    int64_t value;
    int base;
    assert(dstring_at(raw_dstr, 0) == '\\');
    get_escape_sequence(raw_dstr, &value, &base);
    Token* result = malloc(sizeof(Token));
    result->i_value = value;
    result->type = CONSTANT;
    result->constant_type = CHARAC;
    memcpy(&result->text, &scanned, sizeof(dstring));
    return result;
}

Token* scan_char_non_escaped(dstring* scanned)
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

Token* scan_char_constant(FILE* fptr)
{
    // only looks for closing ' until \n or eof (or \r?)
    // for numerical escape sequences, range is 0-255
    // hex: \xff
    // octal: \377
    int ch;
    Token* result = NULL;

    dstring scanned;
    dstring_reserve(&scanned, 8);

    // assume unescaped and no size modifier
    int i = 1;
    char size_modifier = '\0';
    bool escaped = false;

    ch = fgetc(fptr);
    if (ch == EOF) return NULL;
    dstring_append(&scanned, ch);
    if (ch == 'l' || ch == 'L' || ch == 'u' || ch == 'u') {
        size_modifier = ch;
        ++i;

        ch = fgetc(fptr); // read the opening '
        dstring_append(&scanned, ch);
    }

    ch = fgetc(fptr);
    dstring_append(&scanned, ch);
    if (ch == '\\') {
        escaped = true;
    }

    // look for closing '
    while ((ch = fgetc(fptr)) != '\'') {
        if (ch == EOF || ch == '\n') {
            fprintf(stderr, "err: missing closing '\n");
            exit(1);
        }
        dstring_append(&scanned, ch);
    }
    dstring_append(&scanned, '\'');

    if (escaped) {
        dstring raw_dstr;
        dstring_initialize_str(&raw_dstr, scanned.str+i, -1);
        result = scan_char_escape_sequence(&scanned, &raw_dstr, size_modifier);
    } else {
        result = scan_char_non_escaped(&scanned);
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
        result = scan_char_constant(fptr);
    }

    return result;
}

Token* scan_string_literal(FILE* fptr)
{
    Token* result = NULL;

    char ch0 = fgetc(fptr);
    char ch1 = fgetc(fptr);
    char ch2 = fgetc(fptr);

    if (ch0 == '"' ||
            (ch0 == 'u' && ch1 == '8' && ch2 == '"') ||
            ((ch0 == 'u' || ch0 == 'U' || ch0 == 'L') && ch1 == '"')) {

        // grab the whole str and put it in a char*
        // need to verify every char in the str is a valid s-char
        dstring scanned;
        dstring_initialize(&scanned);

        dstring_append(&scanned, ch0);
        dstring_append(&scanned, ch1);
        dstring_append(&scanned, ch2);

        while ((ch0 = fgetc(fptr)) != '"') {
            if (ch0 == '\n' || ch0 == '\r' || ch0 == EOF) {
                fprintf(stderr, "err: missing terminating \"\n");
                exit(1);
            }

            dstring_append(&scanned, ch0);
        }
        dstring_append(&scanned, ch0);

        dstring raw_dstr;
        dstring_initialize(&raw_dstr);

        int i = ch2 == '"' ? 3 : (ch1 == '"' ? 2 : 1);
        while (i < scanned.len-1) {
            if (dstring_at(&scanned, i) == '\\') {
                int64_t ch; int b;
                dstring temp;
                dstring_initialize_str(&temp, scanned.str+i, scanned.len-i-1);

                // careful, it's i+= and not i=
                i += get_escape_sequence(&temp, &ch, &b);

                dstring_append(&raw_dstr, (char)ch);
                dstring_free(temp);
            } else {
                dstring_append(&raw_dstr, dstring_at(&scanned, i++));
            }
        }
        result = malloc(sizeof(Token));
        result->type = STRING_LIT;
        memcpy(&result->s_value, &raw_dstr, sizeof(dstring));
        memcpy(&result->text, &scanned, sizeof(dstring));
    }
    else {
        ungetc(ch2, fptr);
        ungetc(ch1, fptr);
        ungetc(ch0, fptr);
    }


    return result;
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
        fprintf(stderr, "err: missing argument: file\n");
        return 1;
    }

    FILE* fptr = fopen(argv[1], "r");

    if (fptr == NULL) {
        fprintf(stderr, "err: file can't be opened\n");
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
