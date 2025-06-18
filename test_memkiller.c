#include <stdio.h>
#include <stdlib.h>

#define SIZE (128 * 1024 * 1024) // 128MB

int main() {
    char *array = malloc(SIZE);
    for (int i = 0; i < SIZE; i += 64) { // 步長 = 1 cache line
        array[i] = i % 256;
    }
    printf("Done\n");
    return 0;
}
