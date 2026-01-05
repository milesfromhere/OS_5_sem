#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <time.h>

struct ThreadData {
    int id;
    int niceValue;
};

void SetProcessAffinity(unsigned long mask) {
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
        fprintf(stderr, "Affinity error: %s\n", strerror(errno));
}

void* WorkerThread(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;

    if (setpriority(PRIO_PROCESS, 0, data->niceValue) == -1)
        fprintf(stderr, "setpriority error: %s\n", strerror(errno));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    const int TOTAL_ITERS = 1000000;

    for (int i = 1; i <= TOTAL_ITERS; i++) {
        if (i % 1000 == 0) {
            usleep(200000); // 200 ms

            int nice_now = getpriority(PRIO_PROCESS, 0);

            printf("[Поток %d] Итерация = %d\n", data->id, i);
            printf("  PID: %d\n", getpid());
            printf("  TID: %ld\n", (long)syscall(SYS_gettid));
            printf("  Nice: %d\n", nice_now);
            printf("  CPU: %d\n", sched_getcpu());
            printf("--------------------------------------\n");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double secs = (end.tv_sec - start.tv_sec) +
                  (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("[Поток %d] Закончился за %.3f сек\n", data->id, secs);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("./Lab-05c <P1_mask> <P2_thr1_nice> <P3_thr2_nice>\n");
        return 0;
    }

    unsigned long mask = strtoul(argv[1], NULL, 10);
    int nice1 = atoi(argv[2]);
    int nice2 = atoi(argv[3]);

    printf("Параметры\n");
    printf("P1 Mask: %lu\n", mask);
    printf("P2 Thread1 Nice: %d\n", nice1);
    printf("P3 Thread2 Nice: %d\n\n", nice2);

    SetProcessAffinity(mask);

    pthread_t t1, t2;
    struct ThreadData d1 = {1, nice1};
    struct ThreadData d2 = {2, nice2};

    pthread_create(&t1, NULL, WorkerThread, &d1);
    pthread_create(&t2, NULL, WorkerThread, &d2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\nВсе потоки закончены\n");
    return 0;
}
