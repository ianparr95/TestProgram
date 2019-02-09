#include "FixedSizeMoveToFrontHashMap.h"
#include "mman.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: create an index structure. (for indexes and their location in the file).

void ReadHashTableIntoMemory()
{
    /*
    int fd = open("table_001.hx",O_RDWR);
    int tableSize;
    read(fd, &tableSize, sizeof(int));
    printf("\r\nWe read the table size, it is: %d\r\n", tableSize);
    FILE* file = fdopen(fd, "r");
    fseek(file, 0L, SEEK_END);
    int totalFileSize = ftell(file) - sizeof(int);
    printf("\r\nTotal file size is: %d", totalFileSize);
    fseek(file, sizeof(int), SEEK_SET); // seek back to after we read the int

    fclose(file);*/
    // TODO: probably don't need this: just have an index structure to seek the file.
}

void WriteHashTableToDisk(FixedSizeHashTable table, long tableSize)
{
    printf("Beginning to write to disk\r\n");
    // Calculate file size.
    size_t metadataSize = 4; // table size only for now.
    size_t totalSize = 0;

    for (long i = 0; i < tableSize; i++)
    {
        if (table[i]->headNode != 0)
        {
            totalSize += table[i]->headNode->keyAndValueSizeInBytes;
            totalSize += (sizeof(short) * table[i]->headNode->numElements); // length of the value (we assume we don't need to store keylen.. for now!).
            totalSize += sizeof(int); // store num elements itself
            totalSize += sizeof(int); // store total size of this row. (ie: the table[index].
        }
        else
        {
            totalSize += sizeof(int); // only store sizeof int ZERO.
        }
    }

    totalSize += metadataSize;

    int expectedSize = metadataSize;

    printf("We got total size of %d\r\n", totalSize);

    int fd = open("table_001.hx", O_RDWR | O_CREAT, (mode_t)0600);

    char *map = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Need some space to write metadata.
    // TODO:
    memcpy(map, &tableSize, metadataSize);

    long offset = metadataSize;
    for (long i = 0; i < tableSize; i++)
    {
        int curDepth = 0;
        if (table[i]->headNode != 0)
        {
            expectedSize += table[i]->headNode->keyAndValueSizeInBytes;
            expectedSize += (sizeof(short) * table[i]->headNode->numElements); // length of the value (we assume we don't need to store keylen.. for now!).
            expectedSize += sizeof(int); // store num elements itself
            expectedSize += sizeof(int); // store total size of this row. (ie: the table[index].
            if (i < 10) {
                printf("We're at i = %d, offset = %d, %d, total size was %d\r\n", i, offset, table[i]->headNode->numElements, totalSize);
            }
            Node* node = table[i]->headNode->head;
            int totalRowSizeOffset = offset;
            int totalRowSize = sizeof(int); // the number of elements.

            offset += sizeof(int); // skip ahead to allow to write the total size of row, after we calculate it.
            memcpy(map + offset, &(table[i]->headNode->numElements), sizeof(int)); // write the number of elements here.
            offset += sizeof(int);
            while(node)
            {
                curDepth++;
                totalRowSize += (KEYLEN + sizeof(short) + node->valueLen); // key + len of val + value.
                if (offset + totalRowSize > totalSize) {
                    printf("We exceeded at %d!\r\n", i);
                }
                memcpy(map + offset, &(node->key), KEYLEN); // write key
                memcpy(map + offset + KEYLEN, &(node->valueLen), sizeof(short)); // write value len
                memcpy(map + offset + KEYLEN + sizeof(short), node->value, node->valueLen); // write value.
                offset += (node->valueLen + KEYLEN + sizeof(short));
                //printf("We're at i = %d, offset = %d, %d, expected: %d, total size was %d\r\n", i, offset, table[i]->headNode->numElements, expectedSize, totalSize);
                if (offset != expectedSize) {
                    printf("They did not match at i:%d expected:%d offset:%d depth:%d, val: %s, len: %d, kvlen: %d\r\n", i, expectedSize, offset, curDepth, node->value, node->valueLen, table[i]->headNode->keyAndValueSizeInBytes);
                    return;
                }
                node = node->next;
            }
            // now rewrite the len of total row.
            memcpy(map + totalRowSizeOffset, &totalRowSize, sizeof(int));
        }
        else
        {
            // this node was not initialized, we still want to write a zero here, cause we assume our tables are not sparsely populated.
            // TODO: think of a better plan (or use RLE compression etc.)
            memset(map + offset, 0, sizeof(int));
            offset += sizeof(int);
            expectedSize += sizeof(int);
        }
    }

    printf("We're done with offset: %d out of total size: %d expected: %d\r\n", offset, totalSize, expectedSize);

    //memcpy(map, text, strlen(text));
    //memcpy((char*)map + 11, text, strlen(text));
    //msync(map, textsize, MS_SYNC);
    munmap(map, totalSize);
    close(fd);

    //return 0;
}
