// General Libraries
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Self Library
#include "Person.h"
#include "Virus.h"

// VIRUS functions
bool isempty(struct TrieNode *root) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root->children[i] != NULL) {
            return false;
        }
    }
    return true;
}

void insert(struct TrieNode *root, const char *key) {
    if (virusesCount >= 50) return; 
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            current->children[index] = createNode();
        }
        current = current->children[index];
    }
    current->isEndEfWord = true;
    virusesCount++;
}

struct TrieNode *createNode() {
    struct TrieNode *node = (struct TrieNode *)malloc(sizeof(struct TrieNode));
    node->isEndEfWord = 0;
    for (int i = 0; i < 26; i++) {
        node->children[i] = NULL;
    }
    return node;
}

struct TrieNode* search(struct TrieNode *root, const char *key) {
    struct TrieNode *current = root;
    for (int i = 0; i < strlen(key); i++) {
        int index = key[i] - 'a';
        if (current->children[index] == NULL) {
            return false;
        }
        current = current->children[index];
    }
    // return (current != NULL && current->isEndEfWord);
    if (current != NULL && current->isEndEfWord)
        return current;
    return NULL;
}

struct TrieNode *deletehelper(struct TrieNode *root, const char *key, int depth) {
    if (root == NULL) {
        return NULL;
    }
    if (depth == strlen(key)) {
        if (root->isEndEfWord) {
            root->isEndEfWord = false;
        }
        if (isempty(root)) {
            free(root);
            root = NULL;
        }
        return root;
    }
    int index = key[depth] - 'a';
    root->children[index] = deletehelper(root->children[index], key, depth + 1);
    if (isempty(root) && !root->isEndEfWord) {
        free(root);
        root = NULL;
    }
    return root;
}

void deletekey(struct TrieNode *root, const char *key) {
    deletehelper(root, key, 0);
}
