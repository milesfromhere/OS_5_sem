#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

void PrintPriorityName(int niceVal) {
    if (niceVal < -10)        printf("HIGH");
    else if (niceVal < 0)     printf("ABOVE NORMAL");
    else if (niceVal == 0)    printf("NORMAL");
    else if (niceVal <= 10)   printf("BELOW NORMAL");
    else                      printf("IDLE");
}

void SetAffinity(unsigned long mask) {
    cpu_set_t set;
    CPU_ZERO(&set);

    if (mask == 0) {
        long cpus = sysconf(_SC_NPROCESSORS_ONLN);
        for (int i = 0; i < cpus; i++)
            CPU_SET(i, &set);
    } else {
        for (int i = 0; i < 64; i++) {
            if (mask & (1UL << i))
                CPU_SET(i, &set);
        }
    }

    if (sched_setaffinity(0, sizeof(set), &set) == -1)
        fprintf(stderr, "Ошибка установки привязки CPU: %s\n", strerror(errno));
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("./Task3 <P1_mask> <P2_nice> <P3_nice>\n");
        return 0;
    }

    unsigned long mask = strtoul(argv[1], NULL, 10);
    int nice1 = atoi(argv[2]);
    int nice2 = atoi(argv[3]);

    printf("Параметры запуска:\n");
    printf("Маска CPU: %lu\n", mask);

    printf("Приоритет процесса 1: ");
    PrintPriorityName(nice1);
    printf("\n");

    printf("Приоритет процесса 2: ");
    PrintPriorityName(nice2);
    printf("\n\n");

    const char* childApp = "lab-05x";

    pid_t pid1 = fork();
    if (pid1 == 0) {
        nice(nice1);
        SetAffinity(mask);
        execl(childApp, childApp, NULL);
        fprintf(stderr, "Ошибка запуска процесса: %s\n", strerror(errno));
        return 1;
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        nice(nice2);
        SetAffinity(mask);
        execl(childApp, childApp, NULL);
        fprintf(stderr, "Ошибка запуска процесса: %s\n", strerror(errno));
        return 1;
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    printf("\nВсе дочерние процессы завершены.\n");
    printf("PID процесса 1: %d\n", pid1);
    printf("PID процесса 2: %d\n", pid2);

    return 0;
}
