#include <pthread.h>
#include <stdint.h>

typedef struct Node
{
    struct Node* next;
    void* key;
    void* value;
    uint16_t valueLen;
} Node;

typedef struct HeadNode
{
    struct Node* head;
    uint32_t numElements;
    uint32_t keyAndValueSizeInBytes;
} HeadNode;

typedef struct HeadNodeAndLock
{
    HeadNode* headNode;
    pthread_mutex_t* lock;
} HeadNodeAndLock;

typedef HeadNodeAndLock** FixedSizeHashTable;

FixedSizeHashTable CreateFixedSizeHashMap(long);

void Insert(FixedSizeHashTable, void*, void*, short, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));
int Get(FixedSizeHashTable, void**, short*, void*, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));

extern short KEYLEN;

extern int TotalInserts;
