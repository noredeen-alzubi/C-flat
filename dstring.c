#include "cflat.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void dstring_append(dstring* dstr, char c)
{
    if (dstr->len + 1 >= dstr->size) {
        dstr->size = dstr->size * 2 + 1;
        dstr->str = realloc(dstr->str, sizeof(char) * dstr->size);
    }

    dstr->str[dstr->len++] = c; // overwrite \0
    dstr->str[dstr->len] = '\0';
}

char dstring_at(dstring* dstr, int i)
{
    if (i >= dstr->len || i < 0) {
        fprintf(stderr, "internal err: index out of range\n");
        exit(3);
    }
    return dstr->str[i];
}

// TODO: questionable function. don't use
char dstring_set(dstring* dstr, int i, char c)
{
    if (i >= dstr->len || i < 0) {
        fprintf(stderr, "internal err: index out of range\n");
        exit(3);
    }
    dstr->str[i] = c;
    return c;
}

void dstring_initialize(dstring* dstr)
{
    dstr->str = malloc(1);
    dstr->str[0] = '\0';
    dstr->size = 1;
    dstr->len = 0;
}

void dstring_reserve(dstring* dstr, size_t len)
{
    dstr->size = len + 1;
    dstr->str = malloc(dstr->size);
    dstr->str[0] = '\0';
    dstr->len = 0;
}

void dstring_initialize_str(dstring* dstr, char* str)
{
    dstr->len = strlen(str);
    dstr->size = dstr->len + 1;
    dstr->str = malloc(dstr->size);
    strcpy(dstr->str, str);
}

void dstring_cat(dstring* dest, dstring* src)
{
    size_t needed_size = dest->len + src->len + 1;
    if (dest->size < needed_size) {
        dest->size = 2 * needed_size;
        dest->str = realloc(dest->str, dest->size);
    }

    for(int i = 0; i < src->len; ++i) {
        dest->str[dest->len++] = src->str[i];
    }
    dest->str[dest->len] = '\0';
}
