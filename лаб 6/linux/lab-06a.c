#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/syscall.h>
#include <string.h>

pthread_mutex_t posixMutex = PTHREAD_MUTEX_INITIALIZER;

struct ThreadParameters {
    int iterationCount;
    const char* threadIdentifier;
};

void* ThreadWorker(void* parameter) {
    struct ThreadParameters* threadData = (struct ThreadParameters*)parameter;
    int iterationCount = threadData->iterationCount;
    const char* threadIdentifier = threadData->threadIdentifier;

    struct passwd* userInfo = getpwuid(getuid());
    const char* userName = userInfo ? userInfo->pw_name : "Unknown";
    int userNameSize = (int)strlen(userName);

    pid_t threadId = syscall(SYS_gettid);
    
    int isThreadA = (strcmp(threadIdentifier, "A") == 0);
    int isThreadB = (strcmp(threadIdentifier, "B") == 0);
    int isThreadM = (strcmp(threadIdentifier, "M") == 0);

    // Потоки A и M завершают работу на 60 итерации, B продолжает до 90
    int actualIterations = iterationCount;
    if (isThreadA || isThreadM) {
        actualIterations = 60; // A и M завершаются на 60 итерации
    }

    for (int iteration = 1; iteration <= actualIterations; iteration++) {
        // Оригинальная логика с мьютексом
        if (iteration == 30)
            pthread_mutex_lock(&posixMutex);

        printf("PID=%d | TID=%d | Поток=%s | Итерация=%d | Символ=%c",
               getpid(),
               (int)threadId,
               threadIdentifier,
               iteration,
               userName[(iteration - 1) % userNameSize]);
        
        // Добавляем информацию о завершении
        if ((isThreadA || isThreadM) && iteration == 60) {
            printf(" | [ЗАВЕРШАЕТСЯ]");
        }
        
        // Информация для потока B после 60 итерации
        if (isThreadB && iteration >= 60) {
            printf(" | [РАБОТАЕТ ОДИН]");
        }
        
        printf("\n");

        if (iteration == 60)
            pthread_mutex_unlock(&posixMutex);

        usleep(100 * 1000); // 100 ms
        
        // Завершение потоков A и M на 60 итерации
        if ((isThreadA || isThreadM) && iteration == 60) {
            printf("Поток %s завершил работу на 60 итерации\n", threadIdentifier);
            break;
        }
    }
    
    // Дополнительные итерации для потока B (если он продолжает работу)
    if (isThreadB && iterationCount > 60) {
        printf("\n=== Поток B продолжает работу один (итерации 61-90) ===\n");
        for (int iteration = 61; iteration <= iterationCount; iteration++) {
            printf("PID=%d | TID=%d | Поток=B | Итерация=%d | Символ=%c | [РАБОТАЕТ ОДИН]\n",
                   getpid(),
                   (int)threadId,
                   iteration,
                   userName[(iteration - 1) % userNameSize]);
            usleep(100 * 1000);
        }
    }

    return NULL;
}

int main(void) {
    pthread_t pthreadHandles[3];

    // Потоки A и M выполняют 60 итераций, B - 90
    struct ThreadParameters threadParamA = {60, "A"};
    struct ThreadParameters threadParamB = {90, "B"};
    struct ThreadParameters threadParamMain = {60, "M"};

    printf("=== Запуск программы ===\n");
    printf("Логика работы:\n");
    printf("1. Итерации 1-60: работают все потоки (A, B, M)\n");
    printf("2. Итерация 60: потоки A и M завершают работу\n");
    printf("3. Итерации 61-90: работает только поток B\n\n");

    if (pthread_create(&pthreadHandles[0], NULL, ThreadWorker, &threadParamA) != 0 ||
        pthread_create(&pthreadHandles[1], NULL, ThreadWorker, &threadParamB) != 0 ||
        pthread_create(&pthreadHandles[2], NULL, ThreadWorker, &threadParamMain) != 0) {
        fprintf(stderr, "Ошибка создания потоков!\n");
        return 1;
    }

    printf("Созданы потоки A (60 итераций), B (90 итераций) и M (60 итераций).\n\n");

    pthread_join(pthreadHandles[0], NULL);
    pthread_join(pthreadHandles[1], NULL);
    pthread_join(pthreadHandles[2], NULL);

    printf("\n=== Все потоки завершены ===\n");
    printf("Потоки A и M: завершились на 60 итерации\n");
    printf("Поток B: завершился на 90 итерации\n");

    pthread_mutex_destroy(&posixMutex);
    return 0;
}