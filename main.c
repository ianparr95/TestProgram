#include <stdio.h>
#include <stdlib.h>
#include "FixedSizeMoveToFrontHashMap.h"
#include "mtwister.h"

long simpleHash(void* data);
int simpleComparison(void*, void*);
long simpleHash2(long num);

const long IN_MEMORY_SIZE = 1000000;
const long TABLE_SIZE = 100000;
const long MAX_VALUE = 2000000;

const long iterateSize = 100000;

pthread_t tid[10];

void* insertIntoTable(void* arg)
{
    void** args = arg;
    FixedSizeHashTable table = args[0];
    int toIterate = args[1];
    MTRand r = seedRand(args[2]);

    for (long i = toIterate - iterateSize; i < toIterate; i++) {
        Insert(table, (void*)simpleHash2(genRandLong(&r)), (void*)genRandLong(&r), simpleHash, simpleComparison);
    }
}


int main()
{
    FixedSizeHashTable GlobalTable = CreateFixedSizeHashMap(TABLE_SIZE);
    MTRand r = seedRand(time(0));
/*
    for (long i = 0; i < IN_MEMORY_SIZE; i++)
    {

        if (i % 10000 == 0) {
            printf("%d\r\n", i);
        }
        Insert(GlobalTable, (void*)simpleHash2(genRandLong(&r)), (void*)genRandLong(&r), simpleHash, simpleComparison);
    }*/

    int error;
    for (int i = 0 ; i < 10 ; i++)
    {
        void** args = malloc(sizeof(FixedSizeHashTable*) + sizeof(int*) + sizeof(int*));
        args[0] = GlobalTable;
        args[1] = (i+1) * iterateSize;
        args[2] = genRandLong(&r);
        void* arg = args;
        error = pthread_create(&(tid[i]), NULL, &insertIntoTable, arg);
        if (error != 0)
            printf("\nThread can't be created : [%s]", strerror(error));
    }

    for (int i = 0 ; i < 10 ; i ++)
    {
        pthread_join(tid[i], NULL);
    }
    long* value = malloc(sizeof(long));
    int found = 0;
    int notfound = 0;

    for (long i = 0; i < MAX_VALUE; i++)
    {
        int gotValue = Get(GlobalTable, &value, (void*)i, simpleHash, simpleComparison);
        found += gotValue;
    }

    printf("Found: %d Not found: %d \r\n", found, MAX_VALUE - found);

    return 0;
}

long simpleHash(void* data)
{
    return ((long)data % TABLE_SIZE + TABLE_SIZE) % TABLE_SIZE;
}

long simpleHash2(long num)
{
    return (num % MAX_VALUE + MAX_VALUE) % MAX_VALUE;
}

int simpleComparison(void* a, void* b)
{
    return (long)a == (long)b;
}

