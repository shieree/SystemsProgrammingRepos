static char memory[4096];

void* mymalloc (int size, char *file, int line) {
    
    // Initialization that should only run once the first time malloc is called
    if (memory[1] == 0) {
        memory[1] = 4094;
    }

    // This boolean variable dictates when to stop looking for a free chunk
    int foundChunk = 0;
    
    // This variable refers to the array[] index. Starts at 0, the beginning index
    int metadata = 0;
    
    while (foundChunk != 1) {
        
        if (metadata > 4094) {
            // If the current metadata index is greater than 4094, the end of the array has been reached and an error should be returned
            // return error;
        } else if (memory[metadata] == 0 && memory[metadata+1] >= size) {
            // This runs if a free chunk of a sufficient size has been found
            foundChunk = 1;
            int chunkLeftOverSize = memory[metadata+1];
            memory[metadata] = 1;
            memory[metadata+1] = size;
            void* returnPointer = &memory[metadata+2];

            // This places metadata after the end of this new chunk signaling the leftover chunk's size
            if (memory[metadata+memory[metadata+1]+2] == 0 && memory[metadata+memory[metadata+2]+2] == 0) {
                memory[metadata+memory[metadata+2]+2] = chunkLeftOverSize - size;
            }

            // return returnPointer;
        } else if (memory[metadata] == 1) {
            // If the current chunk is already allocated or of an insufficient size, go to the next chunk and continue the loop
            metadata = metadata + memory[metadata+1] + 2;
        }
    }
}

void myfree() {

}

// Meaningless comments to help me get the array indexing math right, you can ignore
// 0   1
// 1   12
// 2   data
// 3   data
// 4   data
// 5   data
// 6   data
// 7   data
// 8   data
// 9   data
// 10  data
// 11  data
// 12  data
// 13  data
// 14  1
// 15  2
// 16  data
// 17  data
// 18  0
// 19  4076