#ifndef TOKEN_TRIE_H
#define TOKEN_TRIE_H

#include <stdlib.h>
#include "lex.h"

#define ALPHABET_SIZE 128

typedef struct TokenTrieNode TokenTrieNode;
struct TokenTrieNode {
    char ch;
    TokenTrieNode* children[ALPHABET_SIZE];
    Token* token;
};

TokenTrieNode* build_token_trie(char** strings, int* subtypes, size_t count, TokenType type);

#endif
