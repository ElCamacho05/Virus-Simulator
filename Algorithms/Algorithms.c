// General Libraries

// Self Library
#include "Algorithms.h"

unsigned int hashFunction(int key, int size) {
    return (unsigned int)key % size;
}