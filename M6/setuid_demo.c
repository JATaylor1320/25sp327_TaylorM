#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/fsuid.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define PRINT_IDS(stage) do { \
    uid_t ruid, euid, suid; \
    if (getresuid(&ruid, &euid, &suid) == -1) { \
        perror("getresuid"); \
        exit(EXIT_FAILURE); \
    } \
    printf(#stage ": RUID=%d, EUID=%d, SUID=%d\n", ruid, euid, suid); \
} while (0)

int main() {
    PRINT_IDS(t0: Running with root privileges);

    // Drop privileges temporarily
    if (seteuid(getuid()) == -1) {
        perror("seteuid");
        return EXIT_FAILURE;
    }
    PRINT_IDS(t1: Dropped privileges temporarily);

    // Restore privileges
    if (seteuid(0) == -1) {
        perror("seteuid");
        return EXIT_FAILURE;
    }
    PRINT_IDS(t2: Restored privileges);

    // Drop privileges again
    if (seteuid(getuid()) == -1) {
        perror("seteuid");
        return EXIT_FAILURE;
    }
    PRINT_IDS(t3: Dropped privileges again);

    // Permanently drop privileges
    printf("t4: Permanently dropping privileges...\n");
    if (setresuid(getuid(), getuid(), getuid()) == -1) {
        perror("setresuid");
        return EXIT_FAILURE;
    }
    PRINT_IDS(t4: After permanent drop);

    // Attempt to restore privileges
    printf("t5: Attempting to restore privileges...\n");
    if (seteuid(0) == -1) {
        perror("seteuid");
        fprintf(stderr,"t5: Failed to restore privileges: %s\n", strerror(errno));
    } else {
        PRINT_IDS(t5: Restored privileges);
    }

    return EXIT_SUCCESS;
}
