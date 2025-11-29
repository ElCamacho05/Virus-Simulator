#ifndef VIRUS_H
#define VIRUS_H

// VIRUS constants
#define ALPHABET_SIZE 27 // alphabet
#define MAX_VIRUSES 100 // 50 ORIGINALLY

// VIRUS structures
typedef struct Virus{
    int id;
    char name[20];
    double beta; // ??
    double caseFatalityRatio;
    double recovery; // ??
} VIRUS;

struct TrieNode {
    struct TrieNode *children[26];
    bool isEndEfWord;
    VIRUS *virus;
};

// VIRUSES variables
int virusesCount = 0;

// VIRUS functions
bool isempty(struct TrieNode *root);
void insert(struct TrieNode *root, const char *key);
struct TrieNode *createNode();
struct TrieNode* search(struct TrieNode *root, const char *key);
struct TrieNode *deletehelper(struct TrieNode *root, const char *key, int depth);
void deletekey(struct TrieNode *root, const char *key);

#endif