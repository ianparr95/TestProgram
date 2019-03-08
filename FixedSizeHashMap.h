#include <pthread.h>
#include <stdint.h>

typedef struct Node
{
    struct Node* next;
    void* key;
    uint16_t keyLen;
    void* value;
    uint16_t valueLen;
} Node;

typedef struct HeadNode
{
    struct Node* head;
    uint32_t numElements;
    uint32_t keyAndValueSizeInBytes; // basically keyLen + valueLen, but this is NOT the size of the node itself.
} HeadNode;

typedef struct HeadNodeAndLock
{
    HeadNode* headNode;
    pthread_mutex_t* lock;
} HeadNodeAndLock;

typedef HeadNodeAndLock** FixedSizeHashTable;

FixedSizeHashTable CreateFixedSizeHashMap(long);

void Insert(FixedSizeHashTable, void*, uint16_t, void*, uint16_t, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));

// TODO: don't use this GET FUNCTION.
int Get(FixedSizeHashTable, void**, short*, void*, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));

extern short KEYLEN;

extern int TotalInserts;
