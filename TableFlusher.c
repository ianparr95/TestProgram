#include "FixedSizeMoveToFrontHashMap.h"
#include "mman.h"
#include <fcntl.h>
#include <stdio.h>

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
        }
        else
        {
            totalSize += sizeof(int); // only store sizeof int ZERO.
        }
    }

    totalSize += metadataSize;

    printf("We got total size of %d\r\n", totalSize);

    int fd = open("table_001.hx", O_RDWR | O_CREAT, (mode_t)0600);

    char *map = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Need some space to write metadata.
    // TODO:
    memcpy(map, &tableSize, metadataSize);

    long offset = metadataSize;
    for (long i = 0; i < tableSize; i++)
    {
        if (i % 1000 == 0) {
        printf("We're at i = %d, offset = %d, total size was %d\r\n", i, offset, totalSize);
        }
        if (table[i]->headNode != 0)
        {
            Node* node = table[i]->headNode->head;
            memcpy(map + offset, &(table[i]->headNode->numElements), sizeof(int)); // write the number of elements here.
            offset += sizeof(int);
            while(node)
            {
                memcpy(map + offset, &(node->key), KEYLEN); // write key
                memcpy(map + offset + KEYLEN, &(node->valueLen), sizeof(short)); // write value len
                memcpy(map + offset + KEYLEN + sizeof(short), node->value, node->valueLen); // write value.
                offset += (node->valueLen + KEYLEN + sizeof(short));
                node = node->next;
            }
        }
        else
        {
            // this node was not initialized, we still want to write a zero here, cause we assume our tables are not sparsely populated.
            // TODO: think of a better plan (or use RLE compression etc.)
            memset(map + offset, 0, sizeof(int));
            offset += sizeof(int);
        }
    }

    printf("We're done with offset: %d out of total size: %d\r\n", offset, totalSize);

    //memcpy(map, text, strlen(text));
    //memcpy((char*)map + 11, text, strlen(text));
    //msync(map, textsize, MS_SYNC);
    munmap(map, totalSize);
    close(fd);

    //return 0;
}
