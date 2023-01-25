#include <string.h>
#include <stdio.h>
#include "token_trie.h"

TokenTrieNode* build_token_trie(char* strings[], int subtypes[], size_t count, TokenType type) {
    TokenTrieNode* root = calloc(1, sizeof(TokenTrieNode));
    root->ch = '\0';

    for (int i = 0; i < count; ++i) {
        TokenTrieNode* curr = root;
        char* str = strings[i];
        int j = 0;
        for (; str[j] != '\0'; ++j) {
            if (!curr->children[str[j]]) {
                TokenTrieNode* new = calloc(1, sizeof(TokenTrieNode));
                new->ch = str[j];
                curr->children[str[j]] = new;
            }
            curr = curr->children[str[j]];
        }
        Token* token = malloc(sizeof(Token));
        token->text = malloc(j + 1); // deep copy string
        strcpy(token->text, str);
        token->type = type;
        if (type == KEYWORD) token->keyword_type = subtypes[i];
        else if (type == PUNCTUATOR) token->punctuator_type = subtypes[i];
        else exit(1);
        curr->token = token;
    }

    return root;
}
