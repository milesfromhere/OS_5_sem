#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <iostream>

void ProcessExecutionCycle(const char* processLabel)
{
    char userNameBuffer[256];
    DWORD bufferSize = 256;
    GetUserNameA(userNameBuffer, &bufferSize);

    HANDLE semaphoreHandle = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, FALSE, "LabSem");
    if (!semaphoreHandle)
        semaphoreHandle = CreateSemaphoreA(NULL, 1, 1, "LabSem");

    // Счетчик завершенных процессов (A и Main)
    HANDLE finishedCounter = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "FinishedCounter");
    volatile LONG* finishedCount = NULL;

    if (!finishedCounter)
    {
        finishedCounter = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, sizeof(LONG), "FinishedCounter");
    }

    if (finishedCounter)
    {
        finishedCount = (volatile LONG*)MapViewOfFile(finishedCounter, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(LONG));
        if (finishedCount && strcmp(processLabel, "Main") == 0)
        {
            *finishedCount = 0; // Инициализация
        }
    }

    int userNameLength = (int)strlen(userNameBuffer);
    bool isMainProcess = (strcmp(processLabel, "Main") == 0);
    bool isProcessA = (strcmp(processLabel, "A") == 0);
    bool isProcessB = (strcmp(processLabel, "B") == 0);

    // Определяем сколько итераций выполнять
    int maxIterations = 90; // Для B
    if (isProcessA || isMainProcess)
    {
        maxIterations = 60; // A и Main завершаются на 60 итерации
    }

    for (int iteration = 1; iteration <= maxIterations; iteration++)
    {
        // Оригинальная логика с семафором для итерации 30
        if (iteration == 30)
            WaitForSingleObject(semaphoreHandle, INFINITE);

        printf("Процесс=%s | Итерация=%d | Символ=%c\n",
            processLabel,
            iteration,
            userNameBuffer[(iteration - 1) % userNameLength]);

        Sleep(100);

        if (iteration == 60)
            ReleaseSemaphore(semaphoreHandle, 1, NULL);

        // Процессы A и Main завершают работу на 60 итерации
        if ((isProcessA || isMainProcess) && iteration == 60)
        {
            printf("Процесс %s завершил 60 итераций\n", processLabel);

            if (finishedCount)
            {
                InterlockedIncrement(finishedCount);
                printf("Завершенных процессов: %d\n", *finishedCount);
            }
            break; // Завершаем выполнение
        }

        // Процесс B после 60 итерации проверяет, завершились ли A и Main
        if (isProcessB && iteration >= 60)
        {
            if (finishedCount && *finishedCount < 2) // Ждем завершения A и Main
            {
                printf("Процесс B ожидает завершения A и Main... (завершено: %d/2)\n", *finishedCount);
                Sleep(200);
                iteration--; // Остаемся на текущей итерации
                continue;
            }

            if (iteration == 60)
            {
                printf("\n=== Процесс B продолжает работу один ===\n");
            }
        }
    }

    // Процесс B работает один после 60 итерации
    if (isProcessB)
    {
        printf("\n=== Только процесс B работает (после 60 итерации) ===\n");
        for (int extraIteration = 61; extraIteration <= 90; extraIteration++)
        {
            printf("Процесс=B | Итерация=%d | Символ=%c | Статус: работает один\n",
                extraIteration,
                userNameBuffer[(extraIteration - 1) % userNameLength]);
            Sleep(100);
        }
    }

    if (finishedCount)
    {
        UnmapViewOfFile((LPVOID)finishedCount);
    }
    if (finishedCounter && !isProcessB)
    {
        CloseHandle(finishedCounter);
    }

    CloseHandle(semaphoreHandle);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Rus");

    if (argc > 1)
    {
        ProcessExecutionCycle(argv[1]);
        system("pause");
        return 0;
    }

    HANDLE semaphoreHandle = CreateSemaphoreA(NULL, 1, 1, "LabSem");

    char executablePath[MAX_PATH];
    GetModuleFileNameA(NULL, executablePath, MAX_PATH);

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfoA, processInfoB;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

    char commandLineA[512];
    char commandLineB[512];

    sprintf(commandLineA, "\"%s\" A", executablePath);
    sprintf(commandLineB, "\"%s\" B", executablePath);

    printf("=== Запуск программы ===\n");
    printf("Логика работы:\n");
    printf("1. Процессы A и Main: выполняют 60 итераций и завершаются\n");
    printf("2. Процесс B: выполняет 90 итераций\n");
    printf("3. После 60 итерации: работает только процесс B\n");
    printf("4. Процесс B ждет завершения A и Main перед продолжением\n\n");

    // Запускаем процессы
    if (!CreateProcessA(NULL, commandLineA, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoA))
    {
        printf("Ошибка запуска процесса A\n");
        return 1;
    }

    Sleep(100); // Небольшая задержка для порядка

    if (!CreateProcessA(NULL, commandLineB, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoB))
    {
        printf("Ошибка запуска процесса B\n");
        return 1;
    }

    ProcessExecutionCycle("Main");

    WaitForSingleObject(processInfoA.hProcess, INFINITE);
    WaitForSingleObject(processInfoB.hProcess, INFINITE);

    CloseHandle(processInfoA.hProcess);
    CloseHandle(processInfoA.hThread);
    CloseHandle(processInfoB.hProcess);
    CloseHandle(processInfoB.hThread);
    CloseHandle(semaphoreHandle);

    printf("\n=== Все процессы завершены ===\n");
    printf("Процессы A и Main завершились на 60 итерации\n");
    printf("Процесс B завершился на 90 итерации\n");
    system("pause");
    return 0;
}