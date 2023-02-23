#include <stdio.h>
#include <stdlib.h>

static char memory[4096];

// Takes in a metadata number and converts the metadata's associated size to an int
int metadataSizeToIntSize(int metadata) {
    return ((memory[metadata+2]*1000) + (memory[metadata+3]*100) + (memory[metadata+4]));
}
// Takes in an int size and the metadata location and places the size into the metadata
void intSizeToMetadataSize(int intSize, int metadata) {
    int thousand = intSize / 1000;
    intSize -= thousand*1000;    
    int hundred = intSize / 100;
    intSize -= hundred*100;
    int tensAndOnes = intSize;

    memory[metadata+2] = thousand;
    memory[metadata+3] = hundred;
    memory[metadata+4] = tensAndOnes;
}
// This checks to see if the current metadata is valid
int isMetadataValid(int metadata) {
    if (memory[metadata] == '~' && memory[metadata+5] == '~') {
        if (memory[metadata+1] == -1 || memory[metadata+1] == 0 || memory[metadata+1] == 1) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
// Function that iterates through the memory's metadata and coalesces free chunks that are adjacent
void coalescing(char *file, int line) {
    int prevMetadata = 0;
    int nextMetadata = metadataSizeToIntSize(prevMetadata) + 6;

    while (nextMetadata <= 4090) {

        // Checking to see if metadata is valid
        int isPrevValid = isMetadataValid(prevMetadata);
        int isNextValid = isMetadataValid(nextMetadata);

        if (isPrevValid == 0) {
            printf("Error in coalescing(): invalid metadata found at memory[%d]. File: %s. Line: %d.\n", prevMetadata, file, line);
            return;
        }
        if (isNextValid == 0) {
            printf("Error in coalescing(): invalid metadata found at memory[%d]. File: %s. Line: %d.\n", nextMetadata, file, line);
            return;
        }

        // This runs if two adjacent chunks are free, combining them and setting the nextMetadata value
        if ((memory[prevMetadata+1] == 0 || memory[prevMetadata+1] == -1) && (memory[nextMetadata+1] == 0 || memory[nextMetadata+1] == -1)) {

            printf("Coalescing(): Joining two free chunks at memory[%d] and memory[%d]\n", prevMetadata, nextMetadata); // debugging print line

            int newChunkSize = metadataSizeToIntSize(prevMetadata) + metadataSizeToIntSize(nextMetadata) + 6;
            for (int i = 0; i < 6; i++) {
                memory[nextMetadata+i] = 0;
            }
            intSizeToMetadataSize(newChunkSize, prevMetadata);
            nextMetadata = metadataSizeToIntSize(prevMetadata) + 6;
        }
        // This runs if the two adjacent chunks are not both free
        else {
            prevMetadata = nextMetadata;
            nextMetadata = nextMetadata + metadataSizeToIntSize(nextMetadata) + 6;
        }
    }
}
// Function that erases the entire array for debugging purposes
void eraseAll() {
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }
}

void *mymalloc (size_t size, char *file, int line) {
    
    // Initialization that should only run once the first time malloc is called
    if (memory[1] == 0 && memory[2] == 0 && memory[3] == 0 && memory[4] == 0) {
        memory[0] = '~';
        memory[2] = 4;
        memory[4] = 90;
        memory[5] = '~';
        
        // Metadata looks like this after initialization
        // 0    ~
        // 1    0
        // 2    4
        // 3    0
        // 4    90
        // 5    ~
    }

    int intSize = (int) size;

    if (intSize < 1) {
        printf("Error in mymalloc(): requested size cannot be less than 1. File: %s. Line: %d.\n", file, line);
        exit(1);
    }

    // This boolean variable dictates when to stop looking for a free chunk
    int foundSuitableChunk = 0;
    
    // This variable refers to the array[] index. Starts at 0
    int metadata = 0;
    
    while (foundSuitableChunk != 1) {
        
        // If the current metadata index is greater than 4090, the end of the array has been reached and an error is returned
        if (metadata >= 4090) {
            printf("Error in mymalloc(): metadata pointer has reached the end of the array. File: %s. Line: %d.\n", file, line);
            exit(1);
        }
        
        int isValidChunk = isMetadataValid(metadata);

        if (isValidChunk == 0) {
            printf("Error in mymalloc(): invalid metadata found at memory[%d]. File: %s. Line: %d.\n", metadata, file, line);
            exit(1);
        }

        int metadataSizeInt = metadataSizeToIntSize(metadata);
        
        // This runs if a free chunk of a sufficient size has been found
        if ((memory[metadata+1] == 0 || memory[metadata+1] == -1) && (metadataSizeInt-6 > intSize)) {

            printf("----------\nmymalloc(): Found suitable chunk at memory[%d]\n", metadata); // debugging print line

            foundSuitableChunk = 1;
            int chunkLeftOverSize = metadataSizeInt - intSize;
            memory[metadata+1] = 1;
            intSizeToMetadataSize(intSize, metadata);
            void* returnPointer = &memory[metadata+6];

            // This places metadata after the end of this new chunk signaling the leftover chunk's size
            // Only runs if the end of the new chunk is not already metadata
            if (memory[metadata+6+intSize] != '~' && memory[metadata+6+intSize+5] != '~') {
                memory[metadata+6+intSize] = '~';
                memory[metadata+7+intSize] = 0;
                intSizeToMetadataSize(chunkLeftOverSize-6, metadata+6+intSize);
                memory[metadata+6+intSize+5] = '~';
            }

            printf("mymalloc(): Returning pointer to memory[%d]\n", metadata+6); // debugging print line

            return returnPointer;
        }
        // This runs if the current chunk is already occupied or if it is unoccupied and of insufficient size
        // Goes to next chunk
        else if (memory[metadata+1] == 1 || ((memory[metadata+1] == 0 || memory[metadata+1] == -1) && metadataSizeInt-6 <= intSize)) {
            metadata = metadata + metadataSizeInt + 6;
        }
        else {
            printf("Error in mymalloc(): expected conditions not met, program failure at end of while loop. File: %s. Line: %d.\n", file, line);
            exit(1);
        }
    }
}

void myfree(void* ptr, char *file, int line) {

    // Casts ptr to a char pointer 'target' for use in myfree()
    char * target = (char *)ptr;

    // Checking if the given pointer actually points to the memory array
    int validPointer = 0;
    for (int i = 0; i < 4096; i++) {
        if (target == &memory[i]) {
            validPointer = 1;
            break;
        }
    }
    if (validPointer == 0) {
        printf("Error in myfree(): Not a valid pointer. File: %s, Line: %d.\n", file, line);
        return;
    }

    // If metadata is valid, then free and call coalescing()
    if (target[-1] == '~' && target[-6] == '~' && target[-5] == 1 ) {
        target[-5] = -1;

        printf("myfree(): Chunk successfully freed!\n"); // debugging print line

        coalescing(file, line);
        return;
    }
    
    // Pointer attempting to free a chunk already freed
    else if (target[-1] == '~' && target[-6] == '~' && target[-5] == -1) {
        printf("Error in myfree(): This chunk has already been freed. File: %s, Line: %d.\n", file, line);
        return;
    }

    // Pointer attempting to free a chunk that is empty and never allocated
    else if (target[-1] == '~' && target[-6] == '~' && target[-5] == 0) {
        printf("Error in myfree(): This chunk was never allocated. File: %s, Line: %d.\n", file, line);
        return;
    }

    // Pointer attempting to free a chunk with a pointer not at the start of a chunk
    else if (target[-1] != '~' || target[-6] != '~') {
        printf("Error in myfree(): Pointer selected not at the start of a chunk. File: %s, Line: %d.\n", file, line);
        return;
    }

    // Catch all invalid pointer for any other cases
    else {
        printf("Error in myfree(): Not a valid pointer. File: %s, Line: %d.\n", file, line);
        return;
    }
}

// 0    ~
// 1    -1
// 2    0
// 3    0
// 4    12
// 5    ~
// 6    data
// 7    data
// 8    data
// 9    data
// 10   data
// 11   data
// 12   data
// 13   data
// 14   data
// 15   data
// 16   data
// 17   data
// 18   ~
// 19   1
// 20   0
// 21   0
// 22   12
// 23   ~
// 24   data PTR2
// 25   data
// 26   data
// 27   data
// 28   data
// 29   data
// 30   data
// 31   data
// 32   data
// 33   data
// 34   data
// 35   data
// 36   ~
