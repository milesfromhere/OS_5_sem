#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <time.h>

int get_nice_value() {
    long kprio = syscall(SYS_getpriority, 0, 0);
    return (int)(kprio - 20);
}

int main(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    const int total_iterations = 1000000;

    for (int i = 1; i <= total_iterations; i++) {
        if (i % 1000 == 0) {
            usleep(200 * 1000); // 200 ms

            pid_t pid = getpid();
            pid_t tid = syscall(SYS_gettid);
            int nice_val = get_nice_value();
            int cpu = sched_getcpu();

            printf("Итерация: %d\n", i);
            printf("PID: %d\n", pid);
            printf("TID: %d\n", tid);
            printf("Уровень любезности: %d\n", nice_val);
            printf("CPU: %d\n", cpu);
            printf("-----------------------------\n");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Реальное время выполнения программы: %.3f сек.\n", elapsed);

    return 0;
}
