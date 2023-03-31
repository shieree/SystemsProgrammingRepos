#include <stdlib.h>
#include <stdio.h>

static char memory[4096];

int metadataSizeToInt(int metadata) {
    return ((memory[metadata+2]*1000) + (memory[metadata+3]*100) + (memory[metadata+4]));
}
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

int main() {

    int metadata = 0;
    
    memory[0] = '~';
    memory[1] = 0;
    memory[2] = 4;
    memory[3] = 0;
    memory[4] = 90;
    memory[5] = '~';
    printf("Before change ------\n");
    for (int i = 1; i < 5; i++) {
        printf("%d\n", memory[i]);
    }

    int newSize = 2611;

    intSizeToMetadataSize(newSize, metadata);

    printf("After change ------\n");
    for (int i = 1; i < 5; i++) {
        printf("%d\n", memory[i]);
    }

    return 0;

}
