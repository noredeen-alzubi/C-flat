#include <string.h>
#include <stdio.h>
#include "cflat.h"

TokenTrieNode* build_token_trie(char* strings[], int subtypes[], size_t count, TokenType type) {
    TokenTrieNode* root = calloc(1, sizeof(TokenTrieNode));
    root->ch = '\0';

    for (int i = 0; i < count; ++i) {
        TokenTrieNode* curr = root;
        char* str = strings[i];
        for (int j = 0; str[j] != '\0'; ++j) {
            if (!curr->children[str[j]]) {
                TokenTrieNode* new = calloc(1, sizeof(TokenTrieNode));
                new->ch = str[j];
                curr->children[str[j]] = new;
            }
            curr = curr->children[str[j]];
        }
        Token* token = malloc(sizeof(Token));
        dstring_initialize_str(&token->text, str, -1);
        token->type = type;
        if (type == TK_KEYWORD) token->keyword_type = subtypes[i];
        else if (type == TK_PUNCT) token->punctuator_type = subtypes[i];
        else exit(1);
        curr->token = token;
    }

    return root;
}
