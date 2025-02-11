#include <stdio.h>
#include <stdlib.h>

int main() {
    void *ptr = malloc(1024);

    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    printf("Memory allocated successfully.\n");

    free(ptr);
    printf("Memory freed.\n");

    return 0;
}
