#include <stdio.h>
#include <stdlib.h>
#include "FixedSizeMoveToFrontHashMap.h"
#include "mtwister.h"

long simpleHash(void* data);
int simpleComparison(void*, void*);
long simpleHash2(long num);

const long IN_MEMORY_SIZE = 10000000;
const long TABLE_SIZE = 10000;
const long MAX_VALUE = 1000000;

int main()
{
    FixedSizeHashTable GlobalTable = CreateFixedSizeHashMap(TABLE_SIZE);
    MTRand r = seedRand(time(0));

    for (long i = 0; i < IN_MEMORY_SIZE; i++)
    {
        if (i % 10000 == 0) {
            printf("%d\r\n", i);
        }
        Insert(GlobalTable, (void*)simpleHash2(genRandLong(&r)), (void*)genRandLong(&r), simpleHash, simpleComparison);
    }

    long* value = malloc(sizeof(long));
    int found = 0;
    int notfound = 0;

    for (long i = 0; i < MAX_VALUE; i++)
    {
        if (i % 1000 == 0) {
            printf("%d\r\n", i);
        }
        int gotValue = Get(GlobalTable, &value, (void*)i, simpleHash, simpleComparison);
        found += gotValue;
    }

    printf("Found: %d Not found: %d\r\n", found, MAX_VALUE - found);

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

