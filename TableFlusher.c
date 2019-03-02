#include "FixedSizeHashMap.h"
#include "mman.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: create an index structure. (for indexes and their location in the file).
// TODO: USE UNSIGNED CHARS!!

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!  TODO: clean up the file descriptors  !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// row structure at one index is at the moment:
// row_len(int)->num_elements(int)->(rows: key(long at the moment)->val_len(short)->val))

void ReadHashTableIntoMemory(long keyHash)
{
    // NOTE: keyHash is the key itself (TODO: have real keys and not just longs).
    int indexFd = open("table_001.indx", O_RDONLY);

    // Now try to read where this row is in the table:
    int hashLocation = sizeof(uint32_t) * keyHash;

    unsigned char lenBuf[sizeof(uint32_t)];
    lseek(indexFd, hashLocation, SEEK_SET);
    read(indexFd, lenBuf, sizeof(uint32_t)); // TODO: refactor reading a long / int!!
    close(indexFd);

    uint32_t rowLocation = lenBuf[0] | (lenBuf[1] << 8) | (lenBuf[2] << 16) | (lenBuf[3] << 24);

    // We got the row location, now we can open the main file for reading, and read from the rowlocation.
    int fd = open("table_001.hx", O_RDONLY);
    lseek(fd, rowLocation, SEEK_SET);

    // Now read the length of the row.
    read(fd, lenBuf, sizeof(uint32_t)); // TODO: refactor reading a long
    uint32_t rowLen = lenBuf[0] | (lenBuf[1] << 8) | (lenBuf[2] << 16) | (lenBuf[3] << 24);

    // Ok got the row len, now we should read the row in memory (AND CACHE IT? TODO).
    unsigned char* rowBuf = malloc(rowLen);
    read(fd, rowBuf, rowLen);

    // read number of elements:
    uint32_t numElements = rowBuf[0] | (rowBuf[1] << 8) | (rowBuf[2] << 16) | (rowBuf[3] << 24);

    int curOffset = 4;
    for (int i = 0 ; i < numElements; i++)
    {
        // compare keys. TODO: have real key and not keyhash.
        uint32_t curKey = rowBuf[curOffset] | (rowBuf[curOffset + 1] << 8) | (rowBuf[curOffset + 2] << 16) | (rowBuf[curOffset + 3] << 24);
        uint16_t valueLen = rowBuf[curOffset + 4] | rowBuf[curOffset + 5];

        if (curKey == keyHash)
        {
            printf("We found the key! Value is:");
            char* val = malloc(valueLen);
            memcpy(val, &rowBuf[curOffset+6], valueLen); // TODO: fix this, use pointers.
            break;
        }

        curOffset += valueLen + 6; // 6 is key + value len itself
    }
    free(rowBuf);
    close(fd);
}

void WriteHashTableToDisk(FixedSizeHashTable table, long tableSize)
{
    // Calculate file size.
    size_t metadataSize = 4; // table size only for now.
    size_t totalSize = 0;

    for (long i = 0; i < tableSize; i++)
    {
        if (table[i]->headNode != 0)
        {
            totalSize += table[i]->headNode->keyAndValueSizeInBytes;
            totalSize += (sizeof(uint16_t) * table[i]->headNode->numElements); // length of the value (we assume we don't need to store keylen.. for now!).
            totalSize += sizeof(uint32_t); // store num elements itself
            totalSize += sizeof(uint32_t); // store total size of this row. (ie: the table[index].
        }
        else
        {
            totalSize += sizeof(uint32_t); // only store sizeof int ZERO.
        }
    }

    totalSize += metadataSize;

    int fd = open("table_001.hx", O_RDWR | O_CREAT, (mode_t)0600);
    int indexFd = open("table_001.indx", O_RDWR | O_CREAT, (mode_t)0600);

    char *map = mmap(0, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // for the index structure
    int indexSize = tableSize * sizeof(uint32_t);
    char *indexMap = mmap(0, indexSize, PROT_READ | PROT_WRITE, MAP_SHARED, indexFd, 0); // seems to work, but have to read bytes carefully! From right to left!

    // Need some space to write metadata.
    memcpy(map, &tableSize, metadataSize);

    uint32_t offset = metadataSize;
    uint32_t indexOffset = 0;
    for (uint32_t i = 0; i < tableSize; i++)
    {
        if (table[i]->headNode != 0)
        {
            Node* node = table[i]->headNode->head;
            uint32_t totalRowSizeOffset = offset;
            uint32_t totalRowSize = sizeof(uint32_t); // the number of elements.

            memcpy(indexMap + indexOffset, &offset, sizeof(uint32_t)); //write the current offset to the index file

            offset += sizeof(uint32_t); // skip ahead to allow to write the total size of row, after we calculate it.
            memcpy(map + offset, &(table[i]->headNode->numElements), sizeof(uint32_t)); // write the number of elements here.
            offset += sizeof(uint32_t);
            while(node)
            {
                totalRowSize += (KEYLEN + sizeof(uint16_t) + node->valueLen); // key + len of val + value.
                memcpy(map + offset, &(node->key), KEYLEN); // write key
                memcpy(map + offset + KEYLEN, &(node->valueLen), sizeof(uint16_t)); // write value len
                memcpy(map + offset + KEYLEN + sizeof(uint16_t), node->value, node->valueLen); // write value.
                offset += (node->valueLen + KEYLEN + sizeof(uint16_t));
                node = node->next;
            }
            // now rewrite the len of total row.
            // THIS DOES NOT INCLUDE THE LEN OF THIS LONG ITSELF!. So if allocating an array that includes this size, need to add sizeof(long)
            memcpy(map + totalRowSizeOffset, &totalRowSize, sizeof(uint32_t));
        }
        else
        {
            // this node was not initialized, we still want to write a zero here, cause we assume our tables are not sparsely populated.
            // TODO: think of a better plan (or use RLE compression etc.)
            memset(map + offset, 0, sizeof(uint32_t));
            memset(indexMap + indexOffset, 0, sizeof(uint32_t));
            offset += sizeof(uint32_t);
        }
        indexOffset += sizeof(uint32_t);
    }

    munmap(indexMap, indexSize);
    munmap(map, totalSize);
    close(fd);

}
