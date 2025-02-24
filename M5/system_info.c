#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 256

void getHostname(char *hostname) {
    FILE *file = fopen("/proc/sys/kernel/hostname", "r");
    if (file == NULL) {
        perror("Error opening /proc/sys/kernel/hostname");
        exit(EXIT_FAILURE);
    }
    fgets(hostname, BUFFER_SIZE, file);
    fclose(file);
    hostname[strcspn(hostname, "\n")] = 0; // Remove trailing newline
}

void getCpuInfo(char *cpuType, char *cpuModel, int *cpuCores) {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL) {
        perror("Error opening /proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    cpuType[0] = '\0';
    cpuModel[0] = '\0';
    *cpuCores = 0;

    while (fgets(line, BUFFER_SIZE, file) != NULL) {
        if (strncmp(line, "vendor_id", 9) == 0) {
            sscanf(line, "vendor_id : %s", cpuType);
        } else if (strncmp(line, "model : ", 8) == 0) {
            sscanf(line, "model : %s", cpuModel);
        } else if (strncmp(line, "cpu cores", 9) == 0) {
            sscanf(line, "cpu cores : %d", cpuCores);
        }
    }

    fclose(file);
}

void getKernelVersion(char *kernelVersion) {
    FILE *file = fopen("/proc/version", "r");
    if (file == NULL) {
        perror("Error opening /proc/version");
        exit(EXIT_FAILURE);
    }
    fgets(kernelVersion, BUFFER_SIZE, file);
    fclose(file);
    kernelVersion[strcspn(kernelVersion, "\n")] = 0;
}

void getUptime(int *days, int *hours, int *minutes, int *seconds) {
    FILE *file = fopen("/proc/uptime", "r");
    if (file == NULL) {
        perror("Error opening /proc/uptime");
        exit(EXIT_FAILURE);
    }

    double uptime;
    if (fscanf(file, "%lf", &uptime) != 1) {
        fprintf(stderr, "Error reading uptime\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    *days = (int)(uptime / (24 * 3600));
    uptime -= (*days) * (24 * 3600);
    *hours = (int)(uptime / 3600);
    uptime -= (*hours) * 3600;
    *minutes = (int)(uptime / 60);
    *seconds = (int)uptime;
}

void getCpuTime(unsigned long long *userTime, unsigned long long *systemTime, unsigned long long *idleTime) {
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    if (fgets(line, BUFFER_SIZE, file) != NULL) {
        sscanf(line, "cpu %llu %*u %llu %llu", userTime, systemTime, idleTime);
    } else {
        fprintf(stderr, "Error reading cpu stat\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void getMemoryInfo(unsigned long long *memTotal, unsigned long long *memAvailable) {
    FILE *file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        perror("Error opening /proc/meminfo");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    *memTotal = 0;
    *memAvailable = 0;

    while (fgets(line, BUFFER_SIZE, file) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %llu kB", memTotal);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %llu kB", memAvailable);
        }
    }

    fclose(file);
}

int main() {
    char hostname[BUFFER_SIZE];
    char cpuType[BUFFER_SIZE];
    char cpuModel[BUFFER_SIZE];
    int cpuCores;
    char kernelVersion[BUFFER_SIZE];
    int days, hours, minutes, seconds;
    unsigned long long userTime, systemTime, idleTime;
    unsigned long long memTotal, memAvailable;

    getHostname(hostname);
    getCpuInfo(cpuType, cpuModel, &cpuCores);
    getKernelVersion(kernelVersion);
    getUptime(&days, &hours, &minutes, &seconds);
    getCpuTime(&userTime, &systemTime, &idleTime);
    getMemoryInfo(&memTotal, &memAvailable);

    printf("----------------------------------------------------------------------------\n");
    printf("                               System Information\n");
    printf("----------------------------------------------------------------------------\n");
    printf("Current Host Name:             %s\n", hostname);
    printf("CPU Type:                      %s\n", cpuType);
    printf("CPU Model:                     %s\n", cpuModel);
    printf("CPU Cores:                     %d\n", cpuCores);
    printf("Kernel Version                %s\n", kernelVersion);
    printf("Memory Configured:             %llu kB\n", memTotal);
    printf("Memory Available:              %llu kB\n", memAvailable);
    printf("Uptime:                        %d:%02d:%02d:%02d\n", days, hours, minutes, seconds);
    printf("Time Spent in User Mode:       %llu ms\n", userTime * 10);
    printf("Time Spent in System Mode:     %llu ms\n", systemTime * 10);
    printf("Time Spent in Idle Mode:       %llu ms\n", idleTime * 10);
    printf("----------------------------------------------------------------------------\n");

    return 0;
}
