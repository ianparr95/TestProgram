#include "FixedSizeMoveToFrontHashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void __INTERNAL_removeNode(HeadNode* headNode, void* key, int (*equalityCheck)(void*, void*));

FixedSizeHashTable CreateFixedSizeHashMap(long size)
{
    return (FixedSizeHashTable)calloc(size, sizeof(HeadNode*)); // we zero it out, to make sure we can see that it isn't initialized.
}

void Insert(FixedSizeHashTable table, void* key, void* value, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*))
{
    long index = hashFunction(key);

    Node* node = malloc(sizeof(Node));
    node->key = key;
    node->value = value;

    if (!table[index])
    {
        // first insert, create a HeadNode

        HeadNode* headNode = malloc(sizeof(HeadNode));
        headNode->head = node;
        node->next = 0;

        pthread_mutex_t lock = malloc(sizeof(pthread_mutex_t));
        headNode->lock = lock;

        table[index] = headNode;

        return;
    }

    // Remove the old key first.
    __INTERNAL_removeNode(table[index], key, equalityCheck);

    // Not the first insert, so we want to place this to the front.
    // But since we already initialized the HeadNode, we just swap this and the current second.
    node->next = table[index]->head;
    table[index]->head = node;
}

void __INTERNAL_removeNode(HeadNode* headNode, void* key, int (*equalityCheck)(void*, void*))
{
    // Store head node
    Node* temp = headNode->head->next;
    Node* prev = headNode->head;

    // If head node itself holds the key to be deleted
    if (temp != NULL && equalityCheck(temp->key, key))
    {
        prev->next = temp->next; // Changed head
        free(temp); // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && !equalityCheck(temp->key, key))
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) return;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp);  // Free memory
}

// Returns 0 if doesn't exist. Else will populate value and return 1.
int Get(FixedSizeHashTable table, void** value, void* key, long (*hashFunction)(void*), int (*equalityCheck)(void*, void*))
{
    long index = hashFunction(key);

    if (table[index] == 0) return 0;

    Node* node = table[index]->head;
    Node* prev = table[index]->head;

    while(node) // this bucket is initialized, so that's good. Let's just search now.
    {
        if (equalityCheck(node->key, key))
        {
            // Just return it now, and move it to the front.
            *value = node->value;

            if (prev != node)
            {
                prev->next = node->next;
                node->next = table[index]->head;
                table[index]->head = node;
            }
            return 1;
        }
        prev = node;
        node = node->next;
    }

    return 0;
}
