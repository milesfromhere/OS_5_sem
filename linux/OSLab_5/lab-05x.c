#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <sched.h>
#include <time.h>
#include <string.h>
#include <errno.h>

int get_nice_value() {
    errno = 0;
    int prio = getpriority(PRIO_PROCESS, 0);
    if (errno != 0) {
        perror("getpriority");
        return 0;
    }
    return prio;
}

int main(int argc, char* argv[]) {
    // Если передан аргумент - устанавливаем nice из аргумента
    if (argc > 1) {
        int nice_val = atoi(argv[1]);
        errno = 0;
        if (setpriority(PRIO_PROCESS, 0, nice_val) == -1) {
            // Не выводим ошибку, если это обычный пользователь
            // (только root может устанавливать отрицательные nice)
            if (errno != EPERM) {
                fprintf(stderr, "Ошибка setpriority: %s\n", strerror(errno));
            }
        }
    }

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