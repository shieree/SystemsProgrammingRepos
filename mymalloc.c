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

void errorChecking() {

}

void combination() {

}

void* mymalloc (int size, char *file, int line) {
    
    // Initialization that should only run once the first time malloc is called
    if (memory[1] == 0 && memory[2] == 0 && memory[3] == 0 & memory[4] == 0) {
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

    // This boolean variable dictates when to stop looking for a free chunk
    int foundSuitableChunk = 0;
    
    // This variable refers to the array[] index. Starts at 0
    int metadata = 0;
    
    while (foundSuitableChunk != 1) {

        int metadataSizeInt = metadataSizeToIntSize(metadata);
        
        if (metadata > 4090) {
            // If the current metadata index is greater than 4090, the end of the array has been reached and an error should be returned
            // return error;
        } else if (memory[metadata+1] == 0 && metadataSizeInt >= size) {
            // This runs if a free chunk of a sufficient size has been found
            foundSuitableChunk = 1;
            int chunkLeftOverSize = metadataSizeInt - size;
            memory[metadata+1] = 1;
            intSizeToMetadataSize(metadataSizeInt, metadata); // this rewrites the size of the chunk to be the new size
            void* returnPointer = &memory[metadata+6];

            // This places metadata after the end of this new chunk signaling the leftover chunk's size
            if () {
                
            }

            // return returnPointer;
        } else if (memory[metadata] == 1) {
            // If the current chunk is already allocated or of an insufficient size, go to the next chunk and continue the loop
            metadata = metadata + memory[metadata+1] + 2;
        }
    }
}

void myfree(void* ptr, char *file, int line) {

    int validPointer = 0;
    for (int i = 0; i < 4096; i++) {
        if (ptr == memory[i]) {
            validPointer = 1;
        }
    }

    if (validPointer == 0) {
        //error
    }
}

// Meaningless comments to help me get the array indexing math right, you can ignore
// 0    ~
// 1    1
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
// 22   2
// 23   ~
// 24   data
// 25   data
// 26   ~
// 27   0
// 28   4
// 29   0
// 30   64
// 31   ~

// 0    ~
// 1    0
// 2    4
// 3    0
// 5    92
// 6    ~
// 7    