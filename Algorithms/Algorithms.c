// General Libraries

// Self Library
#include "Algorithms.h"

int getHash(int id, int size) {
    id = ((id >> 16) ^ id) * 0x45d9f3b; // Mixing operations
    id = ((id >> 16) ^ id) * 0x45d9f3b;
    id = (id >> 16) ^ id;

    return id % size;
}