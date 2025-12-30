#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

void ExecuteProcessLoop(const char* processLabel)
{
    char userNameBuffer[256];
    DWORD bufferSize = 256;
    GetUserNameA(userNameBuffer, &bufferSize);

    HANDLE mutexHandle = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "LabMutex");
    if (mutexHandle == NULL)
        mutexHandle = CreateMutexA(NULL, FALSE, "LabMutex");

    // Для процесса B создаем событие, которое будет сигнализировать о завершении A и Main
    HANDLE processesDoneEvent = OpenEventA(EVENT_ALL_ACCESS, FALSE, "ProcessesDone");
    if (processesDoneEvent == NULL)
        processesDoneEvent = CreateEventA(NULL, TRUE, FALSE, "ProcessesDone");

    int userNameLength = strlen(userNameBuffer);
    bool isProcessB = (strcmp(processLabel, "B") == 0);
    bool isProcessA = (strcmp(processLabel, "A") == 0);
    bool isMainProcess = (strcmp(processLabel, "Main") == 0);

    for (int iteration = 0; iteration < 100; iteration++) // Увеличим до 100 чтобы видеть работу B после 60
    {
        // Процессы A и Main завершают работу на 60 итерации
        if ((isProcessA || isMainProcess) && iteration == 60)
        {
            printf("Процесс %s завершил работу на итерации 60\n", processLabel);

            // Сигнализируем, что процесс завершил 60 итераций
            HANDLE counterMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "CounterMutex");
            if (counterMutex == NULL)
                counterMutex = CreateMutexA(NULL, FALSE, "CounterMutex");

            HANDLE finishedCountHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "FinishedProcesses");
            int* finishedCount = NULL;

            if (finishedCountHandle == NULL)
            {
                finishedCountHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                    0, sizeof(int), "FinishedProcesses");
            }

            if (finishedCountHandle != NULL)
            {
                finishedCount = (int*)MapViewOfFile(finishedCountHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

                WaitForSingleObject(counterMutex, INFINITE);

                if (finishedCount != NULL)
                {
                    (*finishedCount)++;
                    printf("Завершенных процессов: %d\n", *finishedCount);

                    // Если оба процесса (A и Main) завершились, устанавливаем событие
                    if (*finishedCount >= 2) // A + Main
                    {
                        SetEvent(processesDoneEvent);
                        printf("Все процессы A и Main завершены, процесс B может продолжить работу один\n");
                    }
                }

                ReleaseMutex(counterMutex);

                if (finishedCount != NULL)
                    UnmapViewOfFile(finishedCount);
                CloseHandle(finishedCountHandle);
            }

            CloseHandle(counterMutex);
            break; // Завершаем выполнение процессов A и Main
        }

        // Оригинальная логика с мьютексом для 30-й итерации
        if (iteration == 30)
        {
            WaitForSingleObject(mutexHandle, INFINITE);
        }

        printf("Процесс: %s | Итерация: %d | Символ: %c\n",
            processLabel, iteration, userNameBuffer[iteration % userNameLength]);

        Sleep(100);

        if (iteration == 30)
        {
            ReleaseMutex(mutexHandle);
        }

        // Если это процесс B и он достиг 60 итераций, проверяем, завершились ли другие процессы
        if (isProcessB && iteration >= 60)
        {
            // Если процессы A и Main еще не завершились, ждем
            if (WaitForSingleObject(processesDoneEvent, 0) != WAIT_OBJECT_0)
            {
                printf("Процесс B на итерации %d ждет завершения процессов A и Main...\n", iteration);
                WaitForSingleObject(processesDoneEvent, INFINITE);
                printf("Процесс B продолжает работу один на итерации %d\n", iteration);
            }
        }
    }

    // Процесс B продолжает выполнение после 60 итерации
    if (isProcessB)
    {
        printf("\n=== Процесс B продолжает работу один (после 60 итерации) ===\n");
        for (int extraIteration = 60; extraIteration < 100; extraIteration++)
        {
            printf("Процесс: B | Итерация: %d | Символ: %c | Статус: работает один\n",
                extraIteration, userNameBuffer[extraIteration % userNameLength]);
            Sleep(100);
        }
    }

    CloseHandle(mutexHandle);
    CloseHandle(processesDoneEvent);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Rus");

    if (argc > 1)
    {
        ExecuteProcessLoop(argv[1]);
        system("pause");
        return 0;
    }

    // Инициализируем счетчик завершенных процессов
    HANDLE counterMutex = CreateMutexA(NULL, FALSE, "CounterMutex");
    HANDLE finishedCountHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, sizeof(int), "FinishedProcesses");
    int* finishedCount = NULL;

    if (finishedCountHandle != NULL)
    {
        finishedCount = (int*)MapViewOfFile(finishedCountHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
        if (finishedCount != NULL)
        {
            *finishedCount = 0; // Инициализируем счетчик
        }
    }

    HANDLE mutexHandle = CreateMutexA(NULL, FALSE, "LabMutex");

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
    printf("Процессы A и Main завершатся на 60 итерации\n");
    printf("Процесс B продолжит работу после их завершения\n\n");

    // Запускаем процесс B немного позже, чтобы он мог видеть завершение других
    printf("Запуск процесса A...\n");
    CreateProcessA(NULL, commandLineA, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoA);

    printf("Запуск процесса Main...\n");
    printf("Запуск процесса B...\n");
    CreateProcessA(NULL, commandLineB, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoB);

    ExecuteProcessLoop("Main");

    WaitForSingleObject(processInfoA.hProcess, INFINITE);
    WaitForSingleObject(processInfoB.hProcess, INFINITE);

    CloseHandle(processInfoA.hProcess);
    CloseHandle(processInfoA.hThread);
    CloseHandle(processInfoB.hProcess);
    CloseHandle(processInfoB.hThread);

    if (finishedCount != NULL)
        UnmapViewOfFile(finishedCount);
    CloseHandle(finishedCountHandle);
    CloseHandle(counterMutex);
    CloseHandle(mutexHandle);

    printf("\n=== Все процессы завершены ===\n");
    system("pause");
    return 0;
}