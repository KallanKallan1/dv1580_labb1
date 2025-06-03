#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Each Block represents a portion of the memory pool
typedef struct Block {
    size_t offset;      // Offset in the memory pool
    size_t size;        // Size of the block
    bool free;          // Free status
} Block;

static Block** blockArray = NULL;  // Array of pointers to Block metadata
static size_t blockCapacity = 0;   // Capacity of the block array
static size_t blockCount = 0;      // Number of blocks used
static void* memoryPool = NULL;    // The memory pool

void mem_init(size_t size) {
    // Allocate the memory pool
    memoryPool = malloc(size);

    // Initialize the metadata array
    blockCapacity = 1;  // Initial capacity
    blockArray = malloc(blockCapacity * sizeof(Block*));
    
    // Create the initial free block
    Block* initialBlock = malloc(sizeof(Block));
    initialBlock->offset = 0;
    initialBlock->size = size;
    initialBlock->free = true;

    // Add the initial block to the array
    blockArray[0] = initialBlock;
    blockCount = 1;
}

void* mem_alloc(size_t size) {
    // Search for a free block that meets size requirements
    for (size_t i = 0; i < blockCount; ++i) {
        Block* current = blockArray[i];
        if (current->free && current->size >= size) {
            size_t remainingSize = current->size - size;
            current->free = false;

            // If there's remaining space, create a new free block
            if (remainingSize > 0) {
                // Create a new block for the remaining space
                Block* newBlock = malloc(sizeof(Block));
                newBlock->offset = current->offset + size;
                newBlock->size = remainingSize;
                newBlock->free = true;
                current->size = size;

                // Insert the new block into the array
                if (blockCount >= blockCapacity) {
                    blockCapacity = blockCapacity * 2;
                    blockArray = realloc(blockArray, blockCapacity * sizeof(Block*));
                }
                
                // Shift blocks to make room for the new block
                for (size_t j = blockCount; j > i + 1; --j) {
                    blockArray[j] = blockArray[j - 1];
                }
                // Insert the new free block
                blockArray[i + 1] = newBlock;
                blockCount++;
            }
            // Return a pointer into the memory pool
            return (char*)memoryPool + current->offset;
        }
    }
    // Allocation failed if no suitable free block is found
    return NULL;
}

void mem_free(void* ptr) {
   
    // Find which block corresponds to ptr
    size_t offset = (char*)ptr - (char*)memoryPool;
    Block* current = NULL;
    size_t index = 0;

    // Loop to locate the block by offset
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockArray[i]->offset == offset) {
            current = blockArray[i];
            index = i;
            break;
        }
    }

    // Mark the block free
    current->free = true;

    // Merge with next block if free
    if (index + 1 < blockCount && blockArray[index + 1]->free) {
        Block* next = blockArray[index + 1];
        current->size += next->size;
        free(next);
        // Shift array left to remove next block's pointer
        for (size_t j = index + 1; j < blockCount - 1; ++j) {
            blockArray[j] = blockArray[j + 1];
        }
        blockCount--;
    }

    // Merge with previous block if free
    if (index > 0 && blockArray[index - 1]->free) {
        Block* prev = blockArray[index - 1];
        prev->size += current->size;
        free(current);
        // Shift array left to remove current block's pointer
        for (size_t j = index; j < blockCount - 1; ++j) {
            blockArray[j] = blockArray[j + 1];
        }
        blockCount--;
    }
}

void* mem_resize(void* ptr, size_t size) {
    
    // Locate the block for this pointer
    size_t offset = (char*)ptr - (char*)memoryPool;
    Block* current = NULL;
    size_t index = 0;

    // Loop to find matching offset
    for (size_t i = 0; i < blockCount; ++i) {
        if (blockArray[i]->offset == offset) {
            current = blockArray[i];
            index = i;
            break;
        }
    }
    // If current block is already large enough, reuse it
    if (current->size >= size) {
        return ptr;
    }

    // Attempt to expand into the next block if it's free
    if (index + 1 < blockCount && blockArray[index + 1]->free) {
        Block* next = blockArray[index + 1];
        size_t total_size = current->size + next->size;

        if (total_size >= size) {
            // Merge current block with next free block
            current->size = total_size;
            free(next);
            // Shift the blockArray to remove the next block
            for (size_t j = index + 1; j < blockCount - 1; ++j) {
                blockArray[j] = blockArray[j + 1];
            }
            blockCount--;

            // If there is excess space, split the block
            if (current->size > size) {
                size_t remainingSize = current->size - size;
                // Create a new free block
                Block* newBlock = malloc(sizeof(Block));
                if (!newBlock) {
                    fprintf(stderr, "Failed to allocate block metadata\n");
                    return NULL;
                }
                // Splits the block at the requested size, leaving 'current' allocated 
                // and the remainder in a new free block.
                newBlock->offset = current->offset + size;
                newBlock->size = remainingSize;
                newBlock->free = true;
                current->size = size;
                // Insert the new block into blockArray
                if (blockCount >= blockCapacity) {
                    blockCapacity *= 2;
                    blockArray = realloc(blockArray, blockCapacity * sizeof(Block*));
                }
                // Shift blocks to insert the new free block
                for (size_t j = blockCount; j > index + 1; --j) {
                    blockArray[j] = blockArray[j - 1];
                }
                blockArray[index + 1] = newBlock;
                blockCount++;
            }
            return (char*)memoryPool + current->offset;
        }
    }

    // Cannot resize in place, allocate a new block
    void* newPtr = mem_alloc(size);
    if (newPtr) {
        memcpy(newPtr, ptr, current->size);
        mem_free(ptr);
    }
    return newPtr;
}

void mem_deinit() {
    // Free all block metadata
    for (size_t i = 0; i < blockCount; ++i) {
        free(blockArray[i]);
    }
    free(blockArray);
    blockArray = NULL;
    blockCapacity = 0;
    blockCount = 0;

    // Free the memory pool
    free(memoryPool);
    memoryPool = NULL;
}