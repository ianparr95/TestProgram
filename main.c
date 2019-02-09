#include <stdio.h>
#include <stdlib.h>
#include "FixedSizeMoveToFrontHashMap.h"
#include "mtwister.h"

long simpleHash(void* data);
int simpleComparison(void*, void*);
long simpleHash2(long num);

const long IN_MEMORY_SIZE = 1000000;
const long TABLE_SIZE = 10000000;
const long MAX_VALUE = 10000000;

const long iterateSize = 1000000;

pthread_t tid[10];
//pthread_t tid2[10];

char* bytesRandom[20];

void initializeBytesRandom()
{
    bytesRandom[0] = "a";
    bytesRandom[1] = "qw";
    bytesRandom[2] = "hhh";
    bytesRandom[3] = "fbc4";
    bytesRandom[4] = "abcd4";
    bytesRandom[5] = "fghje5";
    bytesRandom[6] = "124567a";
    bytesRandom[7] = "++++++pg";
    bytesRandom[8] = "deadfishx";
    bytesRandom[9] = "abc123qwej";
    bytesRandom[10] = "abbbbbbbaaf";
    bytesRandom[11] = "aba4123asdgg";
    bytesRandom[12] = "iiiiiiiiiioia";
    bytesRandom[13] = "ggggggggioiog2";
    bytesRandom[14] = "ggaaaagggaaaaaf";
    bytesRandom[15] = "kkkkkkkkkkkkkkkz";
    bytesRandom[16] = "zxczxczxczxczxczx";
    bytesRandom[17] = "OMGWTQQQWEERTBYTET";
    bytesRandom[18] = "BAAAAGGGGGAAGGGaasd";
    bytesRandom[19] = "a23451adoasd0))(()))";
}

void* insertIntoTable(void* arg)
{
    void** args = arg;
    FixedSizeHashTable table = args[0];
    int toIterate = args[1];
    MTRand r = seedRand(args[2]);

    for (long i = toIterate - iterateSize; i < toIterate; i++) {
        Insert(table, (void*)simpleHash2(genRandLong(&r)), (void*)bytesRandom[i % 20], (i % 20) + 1,simpleHash, simpleComparison);
    }
}

void* getFromTable(void* arg)
{
    void** args = arg;
    FixedSizeHashTable table = args[0];
    int toIterate = args[1];
    int* found = malloc(sizeof(int));
    *found = 0;
    for (long i = toIterate - iterateSize; i < toIterate; i++) {
        void* value;
        short valueLen;
        int gotValue = Get(table, &value, (void*)i, &valueLen, simpleHash, simpleComparison);
        *found += gotValue;
    }
    return (void*)found;
}


int main()
{
    FixedSizeHashTable GlobalTable = CreateFixedSizeHashMap(TABLE_SIZE);
    MTRand r = seedRand(time(0));

    initializeBytesRandom();

  //  for (long i = 1; i < 3; i++)
  //  {
  //      printf("Inserting key: %d , value: %d\r\n", i+1, i * 4);
  //      Insert(GlobalTable, /*(void*)simpleHash2(genRandLong(&r))*/i + 1, /*(void*)genRandLong(&r)*/ i * 4, 4, simpleHash, simpleComparison);
  //  }
  //  WriteHashTableToDisk(GlobalTable, TABLE_SIZE);


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
    WriteHashTableToDisk(GlobalTable, TABLE_SIZE);
    ReadHashTableIntoMemory();

/*
    for (long i = 0; i < MAX_VALUE; i++)
    {
        if (i%100000 == 0) printf("i is %d\r\n", i);
        int gotValue = Get(GlobalTable, &value, (void*)i, simpleHash, simpleComparison);
        found += gotValue;
    }
  */
  /*
    for (int i = 0 ; i < 10 ; i++)
    {
        void** args = malloc(sizeof(FixedSizeHashTable*) + sizeof(int*));
        args[0] = GlobalTable;
        args[1] = (i+1) * iterateSize;
        void* arg = args;
        error = pthread_create(&(tid[i]), NULL, &getFromTable, arg);
        if (error != 0)
            printf("\nThread can't be created : [%s]", strerror(error));
    }

    for (int i = 0 ; i < 10 ; i ++)
    {
        void* threadFound = 0;
        pthread_join(tid[i], &threadFound);
        found += *(int*)threadFound;
    }


    printf("Found: %d Not found: %d \r\n", found, MAX_VALUE - found);
*/
printf("Done writing");
        void* value2;
        short valueLen;
        int gotValue = Get(GlobalTable, &value2, &valueLen, (void*)22, simpleHash, simpleComparison);
        if (gotValue) {
            printf("\r\nValue here was: %s with len: %d\r\n", (char*)value2, valueLen);
            char* numStart = (char*)value2;
            for(int l = 0; l < 4; l++) {
                printf("NUM IS: %d %d\r\n", (unsigned char)numStart[l], &numStart[l]);
            }
        } else {
            printf("\r\nWe didnt get!\r\n");
        }
        gotValue = Get(GlobalTable, &value2, &valueLen, (void*)35, simpleHash, simpleComparison);
        if (gotValue) {
            printf("\r\nValue here was: %s with len: %d\r\n", (char*)value2, valueLen);
        } else {
            printf("We didnt get!");
        }
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

