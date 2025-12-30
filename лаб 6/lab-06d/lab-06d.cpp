#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <iostream>

void ProcessExecutionCycle(const char* processIdentifier, HANDLE eventHandle)
{
    char userNameData[256];
    DWORD userNameSize = 256;
    GetUserNameA(userNameData, &userNameSize);

    int userNameLength = (int)strlen(userNameData);
    DWORD processId = GetCurrentProcessId();

    bool isMainProcess = (strcmp(processIdentifier, "Main") == 0);
    bool isProcessA = (strcmp(processIdentifier, "A") == 0);
    bool isProcessB = (strcmp(processIdentifier, "B") == 0);

    // Увеличиваем количество итераций
    int maxIterations = 120; // Чтобы было видно работу после 90

    for (int counter = 1; counter <= maxIterations; counter++)
    {
        // Синхронизация на 1 итерации (как в оригинале)
        if (counter == 1 && !isMainProcess)
            WaitForSingleObject(eventHandle, INFINITE);

        // Синхронизация на 15 итерации
        if (counter == 15 && isMainProcess)
            SetEvent(eventHandle);

        // Главный процесс завершает работу на 90 итерации
        if (isMainProcess && counter == 90)
        {
            printf("PID=%lu | Процесс=%s | Итерация=%d | [ЗАВЕРШАЕТСЯ на 90]\n",
                (unsigned long)processId,
                processIdentifier,
                counter);
            break; // Завершаем Main
        }

        // Вывод информации с указанием статуса
        printf("PID=%lu | Процесс=%s | Итерация=%d | Символ=%c",
            (unsigned long)processId,
            processIdentifier,
            counter,
            userNameData[(counter - 1) % userNameLength]);

        // Добавляем информацию о статусе после 90 итерации
        if (counter >= 90)
        {
            if (isProcessA || isProcessB)
                printf(" | [РАБОТАЕТ после 90]");
            else if (isMainProcess)
                printf(" | [ЗАВЕРШЕН]");
        }

        printf("\n");

        Sleep(100);
    }

    // Ожидание перед закрытием для дочерних процессов
    if (!isMainProcess)
    {
        printf("\nПроцесс %s завершен (работал после 90 итерации).\n", processIdentifier);
        printf("Нажмите любую клавишу для закрытия окна...\n");
        system("pause");
    }
    else
    {
        printf("\nГлавный процесс завершен на 90 итерации.\n");
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Rus");

    HANDLE eventHandle = CreateEventA(NULL, TRUE, FALSE, "Lab06dEvent");

    if (argc > 1)
    {
        ProcessExecutionCycle(argv[1], eventHandle);
        CloseHandle(eventHandle);
        return 0;
    }

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
    printf("1. Итерация 1: A и B ждут события от Main\n");
    printf("2. Итерация 15: Main активирует A и B\n");
    printf("3. Итерация 90: Main завершает работу\n");
    printf("4. Итерации 91-120: работают только A и B\n\n");

    printf("Главный процесс PID=%lu\n", (unsigned long)GetCurrentProcessId());

    // Запускаем процессы A и B
    if (!CreateProcessA(NULL, commandLineA, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoA))
    {
        printf("Ошибка запуска процесса A\n");
        return 1;
    }

    if (!CreateProcessA(NULL, commandLineB, NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfoB))
    {
        printf("Ошибка запуска процесса B\n");
        return 1;
    }

    // Работаем в главном процессе
    ProcessExecutionCycle("Main", eventHandle);

    // Ждем завершения дочерних процессов
    WaitForSingleObject(processInfoA.hProcess, INFINITE);
    WaitForSingleObject(processInfoB.hProcess, INFINITE);

    CloseHandle(processInfoA.hProcess);
    CloseHandle(processInfoA.hThread);
    CloseHandle(processInfoB.hProcess);
    CloseHandle(processInfoB.hThread);
    CloseHandle(eventHandle);

    printf("\n=== Все процессы завершены ===\n");
    printf("Main: завершился на 90 итерации\n");
    printf("A и B: работали до 120 итерации\n");
    printf("\nНажмите любую клавишу для закрытия главного окна...\n");
    system("pause");

    return 0;
}