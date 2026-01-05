#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>

int main(void) {
    pid_t pid = getpid();
    pid_t tid = syscall(SYS_gettid);

    // Класс планировщика
    int policy = sched_getscheduler(0);

    struct sched_param sp;
    sched_getparam(0, &sp);

    // Маска аффинности
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if (sched_getaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        perror("sched_getaffinity");
        return 1;
    }

    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    int current_cpu = sched_getcpu();

    printf("PID процесса: %d\n", pid);
    printf("TID потока: %d\n", tid);

    printf("Класс планировщика: ");
    switch (policy) {
        case SCHED_OTHER:    printf("SCHED_OTHER\n"); break;
        case SCHED_FIFO:     printf("SCHED_FIFO\n"); break;
        case SCHED_RR:       printf("SCHED_RR\n"); break;
#ifdef SCHED_BATCH
        case SCHED_BATCH:    printf("SCHED_BATCH\n"); break;
#endif
#ifdef SCHED_IDLE
        case SCHED_IDLE:     printf("SCHED_IDLE\n"); break;
#endif
#ifdef SCHED_DEADLINE
        case SCHED_DEADLINE: printf("SCHED_DEADLINE\n"); break;
#endif
        default:             printf("Неизвестный (%d)\n", policy); break;
    }

    printf("Приоритет потока: %d\n", sp.sched_priority);

    printf("Маска аффинности процесса (в двоичном виде): ");
    for (int i = 0; i < cpu_count; i++) {
        printf("%d", CPU_ISSET(i, &cpuset) ? 1 : 0);
    }
    printf("\n");

    printf("Количество доступных процессоров: %ld\n", cpu_count);
    printf("Текущий CPU: %d\n", current_cpu);

    return 0;
}
