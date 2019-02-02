#include <pthread.h>

typedef struct Node
{
    struct Node* next;
    void* key;
    void* value;
} Node;

typedef struct HeadNode
{
    struct Node* head;
    int numElements;
} HeadNode;

typedef struct HeadNodeAndLock
{
    HeadNode* headNode;
    pthread_mutex_t* lock;
} HeadNodeAndLock;

typedef HeadNodeAndLock** FixedSizeHashTable;

FixedSizeHashTable CreateFixedSizeHashMap(long);

void Insert(FixedSizeHashTable, void*, void*, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));
int Get(FixedSizeHashTable, void**, void*, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*));
