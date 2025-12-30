#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <locale.h>

void ProcessExecutionCycle(const char* processLabel) {
    struct passwd* userInfo = getpwuid(getuid());
    const char* userName = userInfo ? userInfo->pw_name : "Unknown";
    int userNameLength = (int)strlen(userName);

    // Семафоры для порядка выполнения A->B->M
    sem_t* semA = sem_open("/SemA", O_CREAT, 0644, 1);  // A начинает первым
    sem_t* semB = sem_open("/SemB", O_CREAT, 0644, 0);  // B ждет
    sem_t* semM = sem_open("/SemM", O_CREAT, 0644, 0);  // M ждет
    
    if (semA == SEM_FAILED || semB == SEM_FAILED || semM == SEM_FAILED) {
        perror("sem_open");
        return;
    }

    int isProcessA = (strcmp(processLabel, "A") == 0);
    int isProcessB = (strcmp(processLabel, "B") == 0);
    int isMainProcess = (strcmp(processLabel, "Main") == 0);

    // Определяем количество итераций для каждого процесса
    int maxIterations = 90; // Все делают по 90 итераций

    for (int iteration = 1; iteration <= maxIterations; iteration++) {
        // Процессы A и B завершаются на 60 итерации
        if ((isProcessA || isProcessB) && iteration > 60) {
            break;
        }

        // Синхронизация порядка выполнения (только до 60 итерации)
        if (iteration <= 60) {
            if (isProcessA) {
                sem_wait(semA); // A ждет своей очереди
            } else if (isProcessB) {
                sem_wait(semB); // B ждет своей очереди
            } else if (isMainProcess) {
                sem_wait(semM); // M ждет своей очереди
            }
        }

        printf("Процесс=%s | Итерация=%d | Символ=%c",
               processLabel,
               iteration,
               userName[(iteration - 1) % userNameLength]);

        // Добавляем информацию о статусе
        if (iteration == 60 && (isProcessA || isProcessB)) {
            printf(" | [ЗАВЕРШАЕТСЯ]");
        } else if (iteration > 60 && isMainProcess) {
            printf(" | [РАБОТАЕТ один]");
        }
        printf("\n");

        usleep(50000); // 50 ms

        // Синхронизация передачи очереди (только до 60 итерации)
        if (iteration <= 60) {
            if (isProcessA) {
                sem_post(semB); // После A запускаем B
            } else if (isProcessB) {
                sem_post(semM); // После B запускаем M
            } else if (isMainProcess) {
                sem_post(semA); // После M запускаем A (для следующей итерации)
            }
        }

        // Завершаем процессы A и B на 60 итерации
        if (iteration == 60 && (isProcessA || isProcessB)) {
            printf("Процесс %s завершен на 60 итерации\n", processLabel);
            
            // Освобождаем семафоры для M, чтобы он мог продолжить
            if (isProcessB) {
                // Если это B, нужно дать возможность M продолжить
                sem_post(semM);
            }
        }
    }

    // Процесс M продолжает работу один после 60 итерации
    if (isMainProcess && maxIterations > 60) {
        printf("\n=== Процесс M работает один (итерации 61-90) ===\n");
        for (int iteration = 61; iteration <= maxIterations; iteration++) {
            printf("Процесс=M | Итерация=%d | Символ=%c | [РАБОТАЕТ один]\n",
                   iteration,
                   userName[(iteration - 1) % userNameLength]);
            usleep(50000);
        }
        printf("Процесс M завершил 90 итераций\n");
    }

    // Закрываем семафоры
    sem_close(semA);
    sem_close(semB);
    sem_close(semM);
    
    // Удаляем семафоры только в главном процессе
    if (isMainProcess) {
        sem_unlink("/SemA");
        sem_unlink("/SemB");
        sem_unlink("/SemM");
    }
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Rus");

    if (argc > 1) {
        ProcessExecutionCycle(argv[1]);
        return 0;
    }

    printf("=== Запуск программы ===\n");
    printf("Логика работы:\n");
    printf("1. Итерации 1-60: процессы выполняются по очереди: A, B, M, A, B, M, ...\n");
    printf("2. Итерация 60: процессы A и B завершают работу\n");
    printf("3. Итерации 61-90: работает только процесс M\n\n");

    // Удаляем старые семафоры, если они есть
    sem_unlink("/SemA");
    sem_unlink("/SemB");
    sem_unlink("/SemM");

    // Запускаем процесс A
    pid_t processA = fork();
    if (processA == 0) {
        execlp(argv[0], argv[0], "A", NULL);
        perror("execlp");
        return 1;
    }

    // Запускаем процесс B
    pid_t processB = fork();
    if (processB == 0) {
        execlp(argv[0], argv[0], "B", NULL);
        perror("execlp");
        return 1;
    }

    // Работаем в главном процессе (M)
    ProcessExecutionCycle("Main");

    // Ждем завершения дочерних процессов
    waitpid(processA, NULL, 0);
    waitpid(processB, NULL, 0);

    printf("\n=== Все процессы завершены ===\n");
    printf("Процессы A и B: завершены на 60 итерации\n");
    printf("Процесс M: работал до 90 итерации (один после 60)\n");
    return 0;
}