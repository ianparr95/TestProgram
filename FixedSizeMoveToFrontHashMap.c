#include "FixedSizeMoveToFrontHashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int __INTERNAL_removeNode(HeadNode* headNode, void* key, short* deletedValueLen, int (*equalityCheck)(void*, void*));

FixedSizeHashTable CreateFixedSizeHashMap(long size)
{
    FixedSizeHashTable table = (FixedSizeHashTable)calloc(size, sizeof(HeadNodeAndLock*)); // we zero it out, to make sure we can see that it isn't initialized.
    pthread_mutex_t* locks = calloc(size, sizeof(pthread_mutex_t));
    for(long i = 0; i < size; i++)
    {
        table[i] = calloc(1, sizeof(HeadNodeAndLock));
        table[i]->lock = &locks[i];
    }
    return table;
}

short KEYLEN = 4;

void Insert(FixedSizeHashTable table, void* key, void* value, short valueLen, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*))
{
    long index = hashFunction(key);

    Node* node = malloc(sizeof(Node));
    node->key = key;
    node->value = value;
    node->valueLen = valueLen;

    pthread_mutex_lock(table[index]->lock);
    if (!table[index]->headNode)
    {
        // first insert, create a HeadNode
        table[index]->headNode = malloc(sizeof(HeadNode));
        table[index]->headNode->head = node;
        table[index]->headNode->numElements = 1;
        table[index]->headNode->keyAndValueSizeInBytes = (KEYLEN + valueLen);
        node->next = 0;

        pthread_mutex_unlock(table[index]->lock);
        return;
    }

    // Remove the old key first.
    short deletedValueLen;
    int deleted = __INTERNAL_removeNode(table[index], key, &deletedValueLen, equalityCheck);
    if (!deleted)
    {
        table[index]->headNode->numElements++;
    }
    else
    {
        table[index]->headNode->keyAndValueSizeInBytes -= (KEYLEN + deletedValueLen);
    }

    table[index]->headNode->keyAndValueSizeInBytes += (KEYLEN + valueLen);
    // Not the first insert, so we want to place this to the front.
    // But since we already initialized the HeadNode, we just swap this and the current second.
    node->next = table[index]->headNode->head;
    table[index]->headNode->head = node;
    pthread_mutex_unlock(table[index]->lock);
}

int __INTERNAL_removeNode(HeadNode* headNode, void* key, short* deletedValueLen, int (*equalityCheck)(void*, void*))
{
    // Store head node
    Node* temp = headNode->head->next;
    Node* prev = headNode->head;

    // If head node itself holds the key to be deleted
    if (temp != NULL && equalityCheck(temp->key, key))
    {
        *deletedValueLen = temp->valueLen;
        prev->next = temp->next; // Changed head
        free(temp); // free old head
        return 1;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && !equalityCheck(temp->key, key))
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) return 0;

    *deletedValueLen = temp->valueLen;
    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp);  // Free memory
    return 1;
}

// Returns 0 if doesn't exist. Else will populate value and return 1.
// NO LOCKING HERE? The plan is: can read from tables only flushes (so immutable).
// Problem with the move to front? Alternatively just use an array based.
int Get(FixedSizeHashTable table, void** value, short* valueLen, void* key, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*))
{
    long index = hashFunction(key);

    if (table[index]->headNode == 0)
    {
        return 0;
    }
    Node* node = table[index]->headNode->head;
    Node* prev = table[index]->headNode->head;

    while(node) // this bucket is initialized, so that's good. Let's just search now.
    {
        if (equalityCheck(node->key, key))
        {
            // Just return it now, and move it to the front.
            *value = node->value;
            *valueLen = node->valueLen;
/* // TODO: fix this, with locking or not.
            if (prev != node)
            {
                prev->next = node->next;
                node->next = table[index]->headNode->head;
                table[index]->headNode->head = node;
            }*/
            return 1;
        }
        prev = node;
        node = node->next;
    }

    return 0;
}
